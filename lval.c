#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpc.h"
#include "lval.h"

// Create numeric lval and return pointer
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

// Create error lval and return pointer
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

// Create symbol lval and return pointer
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

lval* lval_qexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

// Properly free all memory allocated for an lval
void lval_del(lval* v) {
  switch (v->type) {
    // Do nothing special for numbers
    case LVAL_NUM: break;

    // Free character buffers storing commands
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;

    case LVAL_QEXPR:
    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      free(v->cell);
    break;
  }
  //Free the lval itself
  free(v);
}

//Add a new s-expression to the chain
lval* lval_add(lval* v, lval* x) {
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

//Build a number lval from an identified numeric in the AST
lval* lval_read_num(mpc_ast_t* t) {
  errno = 0;
  long x = strtol(t->contents, NULL, 10);
  return errno != ERANGE ? lval_num(x) : lval_err("Invalid number");
}

//Parse an lval from the AST
lval* lval_read(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  // if root or sexpr make empty list
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }
  if (strstr(t->tag, "qexpr"))  { x = lval_qexpr(); }

  //Fill list with any valid expressions inside
  for (int i = 0; i < t->children_num; i++) {
    if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
    if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
    if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }

  return x;
}

//Extract element from list at index i
lval* lval_pop(lval* v, int i) {
  lval* x = v->cell[i];

  //Shift memory over location i and decrement list count
  memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));
  v->count--;

  //Reallocate for new sized array and return popped elem
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

//Delete the list after popping the selected element
lval* lval_take(lval* v, int i) {
  lval* x = lval_pop(v, i);
  lval_del(v);
  return x;
}

//Evaluate an s-expression
lval* lval_eval_sexpr(lval* v) {

  //Eval children
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(v->cell[i]);
  }

  //Error checking
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  //Empty expression
  if (v->count == 0) { return v; }

  //Single expression
  if (v->count == 1) { return lval_take(v, 0); }

  //Ensure first element is a symbol
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM) {
    lval_del(f);
    lval_del(v);
    return lval_err("S-expression does not start with symbol.");
  }

  //Call builtin with operator
  lval* result = builtin_op(v, f->sym);
  lval_del(f);
  return result;
}

lval* lval_eval(lval* v) {
  //Evaluate s-expressions
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
  return v;
}

//Compute builtin operators
lval* builtin_op(lval* a, char* op) {
  //Ensure all args are numbers
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Cannot operate on non-number.");
    }
  }

  //Pop top of list
  lval* x = lval_pop(a, 0);

  //Unary negation
  if ((strcmp(op, "-") == 0) && a->count == 0) { x->num = -x->num; }

  //While list has elems, pop and process operators
  while (a->count > 0) {
    lval* y = lval_pop(a, 0);
    
    if (strcmp(op, "+") == 0) { x->num += y->num; }
    if (strcmp(op, "-") == 0) { x->num -= y->num; }
    if (strcmp(op, "*") == 0) { x->num *= y->num; }
    if (strcmp(op, "^") == 0) { x->num = pow(x->num, y->num); }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        return lval_err("Divide by zero.");
      } else {
        x->num /= y->num; 
      }
    }
    if (strcmp(op, "%") == 0) {
      if (y->num == 0) {
        lval_del(x);
        lval_del(y);
        return lval_err("Divide by zero.");
      } else {
        x->num %= y->num; 
      }
    }

    //Done with elem, remove
    lval_del(y);
  }

  //Remove input expression and return result
  lval_del(a);
  return x;
}


void lval_expr_print(lval* v, char open, char close) {
  putchar(open);
  for (int i = 0; i < v->count; i++) {
    //print contents
    lval_print(v->cell[i]);

    //Padding whitespace for all but last element
    if (i != (v->count-1)) {
      putchar(' ');
    }
  }
  putchar(close);
}

// Print contents of lval
void lval_print(lval* v) {
  switch(v->type) {
    case LVAL_NUM: printf("%li", v->num); break;
    case LVAL_ERR: printf("Error: %s", v->err); break;
    case LVAL_SYM: printf("%s", v->sym); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
  }
}

void lval_println(lval* v) {
  lval_print(v);
  putchar('\n');
}
