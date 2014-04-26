#include "builtin.h"

//Macro for error checking in builtin functions
#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

char* ltype_name(ltype_t type) {
  switch(type) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}

//Return the first element in a list
lval* builtin_head(lenv* e, lval* a) {
  LASSERT(a, (a->count == 1), 
      "Function 'head' passed too many arguments. Got %i, expected %i.", a->count, 1);
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
      "Function 'head' passed incorrect type for argument 0. Got %s, expected %s.",
      ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  LASSERT(a, (a->cell[0]->count != 0), "Function 'head' passed {}.");

  lval* v = lval_take(a, 0);
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }
  return v;
}

//Return all but the first element in a list
lval* builtin_tail(lenv* e, lval* a) {
  LASSERT(a, (a->count == 1), 
      "Function 'tail' passed too many arguments. Got %i, Expected %i.", a->count, 1);
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
      "Function 'tail' passed incorrect type for argument 0. Got %s, expected %s.",
      ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  LASSERT(a, (a->cell[0]->count != 0), "Function 'tail' passed {}.");

  lval* v = lval_take(a, 0);
  lval_del(lval_pop(v, 0));
  return v;
}

//Change expression to a list
lval* builtin_list(lenv* e, lval* a) {
  a->type = LVAL_QEXPR;
  return a;
}

//Evaluate a list as an expression
lval* builtin_eval(lenv* e, lval* a) {
  LASSERT(a, (a->count == 1), 
      "Function 'eval' passed too many arguments. Got %i, Expected %i.",
      a->count, 1);
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
      "Function 'eval' passed incorrect type for argument 0. Got %s, expected %s.",
      ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

//Join multiple lists into a single list
lval* builtin_join(lenv* e, lval* a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT(a, (a->cell[i]->type == LVAL_QEXPR),
        "Function 'join' passed incorrect type for argument %i. Got %s, expected %s.",
        i, ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  }

  lval* x = lval_pop(a, 0);

  while (a->count) {
    x = lval_join(x, lval_pop(a, 0));
  }

  lval_del(a);
  return x;
}

//Appends element to beginning of list
lval* builtin_cons(lenv* e, lval* a) {
  LASSERT(a, (a->count == 2), 
      "Function 'cons' passed incorrect number of arguments. Got %i, Expected %i.",
      a->count, 2);
  LASSERT(a, (a->cell[1]->type == LVAL_QEXPR),
      "Function 'cons' passed incorrect type for argument 1. Got %s, Expected %s.",
      ltype_name(a->cell[1]->type), ltype_name(LVAL_QEXPR));

  lval* x = lval_qexpr();
  x = lval_add(x, lval_pop(a, 0));
  x = lval_join(x, lval_pop(a, 0));
  lval_del(a);
  return x;
}

lval* builtin_len(lenv* e, lval* a) {
  LASSERT(a, (a->count == 1), 
      "Function 'len' passed too many arguments. Got %i, Expected %i.",
      a->count, 1);
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
      "Function 'len' passed incorrect type for argument 0. Got %s, Expected %s.",
      ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

  long x = a->cell[0]->count;
  lval_del(a);
  return lval_num(x);
}

lval* builtin_init(lenv* e, lval* a) {
  LASSERT(a, (a->count == 1), 
      "Function 'init' passed too many arguments. Got %i, Expected %i.",
      a->count, 1);
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
      "Function 'init' passed incorrect type for argument 0. Got %s, Expected %s.",
      ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
  LASSERT(a, (a->cell[0]->count != 0), "Function 'init' passed {}.");

  lval* x = lval_take(a, 0);
  lval_pop(x, x->count-1);
  return x;
}

lval* builtin_def(lenv* e, lval* a) {
  LASSERT(a, (a->cell[0]->type == LVAL_QEXPR),
      "Function 'def' passed incorrect type for argument 0. Got %s, expected %s.",
      ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));

  lval* syms = a->cell[0];
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, (syms->cell[i]->type == LVAL_SYM), "Function 'def' cannot define non-symbol, argument %i.", i);
  }

  LASSERT(a, (syms->count == a->count-1), 
      "Function 'def' cannot define incorrect number of values. Got %i, Expected %i.",
      syms->count, a->count-1);

  for (int i = 0; i < syms->count; i++) {
    lenv_put(e, syms->cell[i], a->cell[i+1]);
  }

  lval_del(a);
  return lval_sexpr();
}

lval* builtin_add(lenv* e, lval* a) { return builtin_op(e, a, "+"); }
lval* builtin_sub(lenv* e, lval* a) { return builtin_op(e, a, "-"); }
lval* builtin_mul(lenv* e, lval* a) { return builtin_op(e, a, "*"); }
lval* builtin_pow(lenv* e, lval* a) { return builtin_op(e, a, "^"); }
lval* builtin_div(lenv* e, lval* a) { return builtin_op(e, a, "/"); }
lval* builtin_mod(lenv* e, lval* a) { return builtin_op(e, a, "%"); }

//Compute builtin operators
lval* builtin_op(lenv* e, lval* a, char* op) {
  //Ensure all args are numbers
  for (int i = 0; i < a->count; i++) {
    if (a->cell[i]->type != LVAL_NUM) {
      lval_del(a);
      return lval_err("Function %s cannot operate on non-number, argument %i", op, i);
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
