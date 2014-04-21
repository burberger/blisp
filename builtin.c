#include "builtin.h"

//Macro for error checking in builtin functions
#define LASSERT(args, cond, err) if (!(cond)) { lval_del(args); return lval_err(err); }

//Return the first element in a list
lval* builtin_head(lval* a) {
  LASSERT(a, (a->count == 1), "Function 'head' passed too many arguments.");
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR), "Function 'head' passed incorrect type.");
  LASSERT(a, (a->cell[0]->count != 0), "Function 'head' passed {}.");

  lval* v = lval_take(a, 0);
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }
  return v;
}

//Return all but the first element in a list
lval* builtin_tail(lval* a) {
  LASSERT(a, (a->count == 1), "Function 'tail' passed too many arguments.");
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR), "Function 'tail' passed incorrect type.");
  LASSERT(a, (a->cell[0]->count != 0), "Function 'tail' passed {}.");

  lval* v = lval_take(a, 0);
  lval_del(lval_pop(v, 0));
  return v;
}

//Change expression to a list
lval* builtin_list(lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

//Evaluate a list as an expression
lval* builtin_eval(lval* a) {
  LASSERT(a, (a->count == 1), "Function 'eval' passed too many arguments.");
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR), "Function 'eval' passed incorrect type.");

  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(x);
}

//Join multiple lists into a single list
lval* builtin_join(lval* a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, (a->cell[i]->type == LVAL_QEXPR), "Function 'join' passed incorrect type.");
  }

  lval* x = lval_pop(a, 0);

  while (a->count) {
    x = lval_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

//Appends element to beginning of list
lval* builtin_cons(lval* a) {
  LASSERT(a, (a->count == 2), "Function 'cons' passed incorrect number of arguments.");
  LASSERT(a, (a->cell[1]->type == LVAL_QEXPR), "Fuction 'cons' passed incorrect type.");

  lval* x = lval_qexpr();
  x = lval_add(x, lval_pop(a, 0));
  x = lval_join(x, lval_pop(a, 0));
  lval_del(a);
  return x;
}

lval* builtin_len(lval* a) {
  LASSERT(a, (a->count == 1), "Function 'len' passed too many arguments.");
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR), "Function 'len' passed incorrect type.");

  long x = a->cell[0]->count;
  lval_del(a);
  return lval_num(x);
}

lval* builtin_init(lval* a) {
  LASSERT(a, (a->count == 1), "Function 'init' passed too many arguments.");
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR), "Function 'init' passed incorrect type.");
  LASSERT(a, (a->cell[0]->count != 0), "Function 'init' passed {}.");

  lval* x = lval_take(a, 0);
  lval_pop(x, x->count-1);
  return x;
}

lval* builtin(lval* a, char* func) {
  if (strcmp("list", func) == 0) { return builtin_list(a); }
  if (strcmp("head", func) == 0) { return builtin_head(a); }
  if (strcmp("tail", func) == 0) { return builtin_tail(a); }
  if (strcmp("join", func) == 0) { return builtin_join(a); }
  if (strcmp("eval", func) == 0) { return builtin_eval(a); }
  if (strcmp("cons", func) == 0) { return builtin_cons(a); }
  if (strcmp("len",  func) == 0) { return builtin_len(a);  }
  if (strcmp("init", func) == 0) { return builtin_init(a); }
  if (strstr("+-*^/%", func)) { return builtin_op(a, func); }

  lval_del(a);
  char err[sizeof(func) + 20];
  strcpy(err, "Unknown function: ");
  strcat(err, func);
  return lval_err(err);
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
