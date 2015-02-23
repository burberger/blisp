#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpc.h"
#include "lval.h"
#include "builtin.h"
#include "uthash.h"

//Creates a new environment
lenv* lenv_new(void) {
  lenv* e = malloc(sizeof(lenv));
  e->par = NULL;
  e->vars = NULL;
  return e;
}

//Deletes the provided environment
void lenv_del(lenv* e) {
  struct lvar *current_var, *tmp;

  //Iterate over hash table and clean up each node
  HASH_ITER(hh, e->vars, current_var, tmp) {
    HASH_DEL(e->vars, current_var);
    lval_del(current_var->val);
    free(current_var->sym);
    free(current_var);
  }
  free(e);
}

void lenv_iter(lenv* e) {
  struct lvar *current_var, *tmp;
  //Iterate over hash table and print each node
  HASH_ITER(hh, e->vars, current_var, tmp) {
    printf("%s ", current_var->sym);
    lval_print(current_var->val);
    printf("\n");
  }
}

//Search environment for value
lval* lenv_get(lenv* e, lval* k) {
  struct lvar *result;
  HASH_FIND_STR(e->vars, k->sym, result);
  if (result != NULL) {
    return lval_copy(result->val);
  }
  if (e->par) {
    return lenv_get(e->par, k);
  } else {
    return lval_err("Unbound symbol: %s", k->sym);
  }
}

void lenv_put(lenv* e, lval* k, lval* v) {
  struct lvar *variable;

  //Search table to see if variable exists
  HASH_FIND_STR(e->vars, k->sym, variable);
  //Replace present value if exists
  if (variable != NULL) {
    lval_del(variable->val);
    variable->val = lval_copy(v);
    return;
  }

  //If no existing entry, place new entry in table
  variable = malloc(sizeof(struct lvar));
  variable->sym = malloc(sizeof(k->sym)+1);
  strcpy(variable->sym, k->sym);
  variable->val = lval_copy(v);
  HASH_ADD_STR(e->vars, sym, variable);
}

//Iterates until env has no parent and then defines value globally
void lenv_def(lenv* e, lval* k, lval* v) {
  while(e->par) { e = e->par; }
  lenv_put(e, k, v);
}

lenv* lenv_copy(lenv* e) {
  lenv* n = malloc(sizeof(lenv));
  n->vars = NULL;
  n->par = e->par;

  // Iterate over hashtable and copy each value to new env
  struct lvar *cur_var;
  struct lvar *new_var;
  for (cur_var = e->vars; cur_var != NULL; cur_var = cur_var->hh.next) {
    // Allocate space for new lvar
    new_var = malloc(sizeof(struct lvar));
    new_var->sym = malloc(sizeof(cur_var->sym)+1);
    // Copy values over and add to new table
    strcpy(new_var->sym, cur_var->sym);
    new_var->val = lval_copy(cur_var->val);
    HASH_ADD_STR(n->vars, sym, new_var);
  }

  return n;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
  lval* k = lval_sym(name);
  lval* v = lval_fun(func);
  lenv_put(e, k, v);
  lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
  //List functions
  lenv_add_builtin(e, "list", builtin_list); lenv_add_builtin(e, "len",  builtin_len);
  lenv_add_builtin(e, "head", builtin_head); lenv_add_builtin(e, "tail", builtin_tail);
  lenv_add_builtin(e, "eval", builtin_eval); lenv_add_builtin(e, "join", builtin_join);
  lenv_add_builtin(e, "cons", builtin_cons); lenv_add_builtin(e, "init", builtin_init);

  //Variable functions
  lenv_add_builtin(e, "=",  builtin_put);  lenv_add_builtin(e, "def", builtin_def);
  lenv_add_builtin(e, "\\", builtin_lambda); lenv_add_builtin(e, "env", builtin_env);

  //Control flow functions
  lenv_add_builtin(e, "if", builtin_if);
  lenv_add_builtin(e, "==", builtin_eq); lenv_add_builtin(e, "!=", builtin_ne);
  lenv_add_builtin(e, ">",  builtin_gt); lenv_add_builtin(e, "<",  builtin_lt);
  lenv_add_builtin(e, ">=", builtin_ge); lenv_add_builtin(e, "<=", builtin_le);

  //Logical operators
  lenv_add_builtin(e, "&&", builtin_and); lenv_add_builtin(e, "||", builtin_or);
  lenv_add_builtin(e, "!",  builtin_not);

  //Math functions
  lenv_add_builtin(e, "+", builtin_add); lenv_add_builtin(e, "-", builtin_sub);
  lenv_add_builtin(e, "*", builtin_mul); lenv_add_builtin(e, "/", builtin_div);
  lenv_add_builtin(e, "^", builtin_pow); lenv_add_builtin(e, "%", builtin_mod);

  //String functions
  lenv_add_builtin(e, "load", builtin_load);
  lenv_add_builtin(e, "error", builtin_error);
  lenv_add_builtin(e, "print", builtin_print);
}

// Create numeric lval and return pointer
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

// Create error lval and return pointer
lval* lval_err(char* fmt, ...) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;

  va_list va;
  va_start(va, fmt);

  //Allocate space and then write formatted string to buffer
  v->err = malloc(512);
  vsnprintf(v->err, 511, fmt, va);

  //Reallocate buffer to used space and clean up
  v->err = realloc(v->err, strlen(v->err)+1);
  va_end(va);

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

lval* lval_str(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_STR;
  v->str = malloc(strlen(s)+1);
  strcpy(v->str, s);
  return v;
}

lval* lval_fun(lbuiltin func) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->builtin = func;
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

//Construct a lambda lval
lval* lval_lambda(lval* formals, lval* body) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;

  //Set builtin to null to indicate lambda
  v->builtin = NULL;

  //Build enviornment and set vals
  v->env = lenv_new();
  v->formals = formals;
  v->body = body;
  return v;
}

lval* lval_copy(lval* v) {
  lval* x = malloc(sizeof(lval));
  x->type = v->type;

  switch (v->type) {
    //Copy numbers and functions directly
    case LVAL_NUM: x->num = v->num; break;
    case LVAL_FUN:
      if (v->builtin) {
        x->builtin = v->builtin;
      } else {
        x->builtin = NULL;
        x->env = lenv_copy(v->env);
        x->formals = lval_copy(v->formals);
        x->body = lval_copy(v->body);
      }
    break;

    case LVAL_ERR: x->err = malloc(strlen(v->err)+1); strcpy(x->err, v->err); break;
    case LVAL_SYM: x->sym = malloc(strlen(v->sym)+1); strcpy(x->sym, v->sym); break;
    case LVAL_STR: x->str = malloc(strlen(v->str)+1); strcpy(x->str, v->str); break;

    case LVAL_SEXPR:
    case LVAL_QEXPR:
      x->count = v->count;
      x->cell = malloc(sizeof(lval*) * x->count);
      for (int i = 0; i < x->count; i++) {
        x->cell[i] = lval_copy(v->cell[i]);
      }
    break;
  }

  return x;
}

// Properly free all memory allocated for an lval
void lval_del(lval* v) {
  switch (v->type) {
    // Do nothing special for numbers or functions
    case LVAL_NUM: break;
    case LVAL_FUN:
      if (!v->builtin) {
        lenv_del(v->env);
        lval_del(v->formals);
        lval_del(v->body);
      }
    break;

    // Free character buffers storing commands
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;
    case LVAL_STR: free(v->str); break;

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

lval* lval_read_str(mpc_ast_t* t) {
  int len = strlen(t->contents);
  //Remove closing quote
  t->contents[len-1] = '\0';
  //Length without quotes and with null character is len-1
  char* unescaped = malloc(len-1);
  //Skip first quote character
  strcpy(unescaped, t->contents+1);
  //Find escape characters, make string, clean up
  unescaped = mpcf_unescape(unescaped);
  lval* str = lval_str(unescaped);
  free(unescaped);
  return str;
}

//Parse an lval from the AST
lval* lval_read(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) { return lval_read_num(t); }
  if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
  if (strstr(t->tag, "string")) { return lval_read_str(t); }

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
    if (strstr(t->children[i]->tag,  "comment")) { continue; }
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

//Add each cell in y to x
lval* lval_join(lval* x, lval* y) {
  while (y->count) {
    x = lval_add(x, lval_pop(y, 0));
  }

  lval_del(y);
  return x;
}

lval* lval_call(lenv* e, lval* f, lval* a) {
  // If builtin then apply that
  if (f->builtin) { return f->builtin(e, a); }

  // Store argument counts
  int given = a->count;
  int total = f->formals->count;

  // While arguments remain, process them into env
  while (a->count) {
    // Ran out of formals to bind, return error
    if (f->formals->count == 0) {
      lval_del(a); return lval_err("Function passed too many arguments, Got %i, Expected %i.", given, total);
    }

    // pop first symbol from formals
    lval* sym = lval_pop(f->formals, 0);

    if (strcmp(sym->sym, "&") == 0) {
      //Ensure & is followed by another symbol
      if (f->formals->count != 1) {
        lval_del(a);
        return lval_err("Function formal invalid. Symbol '&' not followed by single symbol.");
      }

      lval* nsym = lval_pop(f->formals, 0);
      lenv_put(f->env, nsym, builtin_list(e, a));
      lval_del(sym); lval_del(nsym);
      break;
    }

    // pop first from args
    lval* val = lval_pop(a, 0);
    // bind copy into function environment
    lenv_put(f->env, sym, val);
    // clean up
    lval_del(sym); lval_del(val);
  }

  // argument list bound, clean up
  lval_del(a);

  if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0) {
    if (f->formals->count != 2) {
      return lval_err("Function format invalid. Symbol '&' not followed by a single symbol");
    }

    lval_del(lval_pop(f->formals, 0));

    lval* sym = lval_pop(f->formals, 0);
    lval* val = lval_qexpr();

    lenv_put(f->env, sym, val);
    lval_del(sym); lval_del(val);
  }

  // If all formals bound, evaluate
  if (f->formals->count == 0) {
    // set function enviornment parent to current eval enviornment
    f->env->par = e;

    return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
  } else {
    // return partially evaluated function
    return lval_copy(f);
  }
}

//Evaluate an s-expression
lval* lval_eval_sexpr(lenv* e, lval* v) {

  //Eval children
  for (int i = 0; i < v->count; i++) {
    v->cell[i] = lval_eval(e, v->cell[i]);
  }

  //Error checking
  for (int i = 0; i < v->count; i++) {
    if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
  }

  //Empty expression
  if (v->count == 0) { return v; }

  //Single expression
  if (v->count == 1) { return lval_take(v, 0); }

  //Ensure element is function after evaluation
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_FUN) {
    lval_del(f);
    lval_del(v);
    return lval_err("First element is not a function.");
  }

  //Call builtin with operator
  lval* result = lval_call(e, f, v);
  lval_del(f);
  return result;
}

lval* lval_eval(lenv* e, lval* v) {
  if (v->type == LVAL_SYM) {
    lval* x = lenv_get(e, v);
    lval_del(v);
    return x;
  }
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
  return v;
}

int lval_eq(lval* x, lval* y) {
  if (x->type != y->type) { return 0; }

  switch (x->type) {
    case LVAL_NUM: return x->num == y->num;
    case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
    case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);
    case LVAL_STR: return (strcmp(x->str, y->str) == 0);
    case LVAL_FUN:
      if (x->builtin || y->builtin) {
        return x->builtin == y->builtin;
      } else {
        return lval_eq(x->formals, y->formals) && lval_eq(x->body, y->body);
      }
    case LVAL_QEXPR:
    case LVAL_SEXPR:
      if (x->count != y->count) { return 0; }
      for (int i = 0; i < x->count; i++) {
        if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
      }
      return 1;
    break;
  }
  return 0;
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

void lval_print_str(lval* v) {
  // Copy the string
  char* escaped = malloc(strlen(v->str)+1);
  strcpy(escaped, v->str);
  // Escape, print between " chars, free copy
  escaped = mpcf_escape(escaped);
  printf("\"%s\"", escaped);
  free(escaped);
}

// Print contents of lval
void lval_print(lval* v) {
  switch(v->type) {
    case LVAL_NUM: printf("%li", v->num); break;
    case LVAL_ERR: printf("Error: %s", v->err); break;
    case LVAL_SYM: printf("%s", v->sym); break;
    case LVAL_STR: lval_print_str(v); break;
    case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
    case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
    case LVAL_FUN:
      if (v->builtin) {
        printf("<builtin>");
      } else {
        printf("(\\ "); lval_print(v->formals); putchar(' '); lval_print(v->body); putchar(')');
      }
    break;
  }
}

void lval_println(lval* v) {
  lval_print(v);
  putchar('\n');
}
