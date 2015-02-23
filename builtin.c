#include "builtin.h"

char* ltype_name(ltype_t type) {
  switch(type) {
    case LVAL_FUN: return "Function";
    case LVAL_NUM: return "Number";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_STR: return "String";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}

//Return the first element in a list
lval* builtin_head(lenv* e, lval* a) {
  LASSERT_NUM("head", a, 1);
  LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("head", a, 0);

  lval* v = lval_take(a, 0);
  while (v->count > 1) {
    lval_del(lval_pop(v, 1));
  }
  return v;
}

//Return all but the first element in a list
lval* builtin_tail(lenv* e, lval* a) {
  LASSERT_NUM("tail", a, 1);
  LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("tail", a, 0);

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
  LASSERT_NUM("eval", a, 1);
  LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);

  lval* x = lval_take(a, 0);
  x->type = LVAL_SEXPR;
  return lval_eval(e, x);
}

//Join multiple lists into a single list
lval* builtin_join(lenv* e, lval* a) {
  for (int i = 0; i < a->count; i++) {
    LASSERT_TYPE("join", a, i, LVAL_QEXPR);
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
  LASSERT_NUM("cons", a, 2);
  LASSERT_TYPE("cons", a, 1, LVAL_QEXPR);

  lval* x = lval_qexpr();
  x = lval_add(x, lval_pop(a, 0));
  x = lval_join(x, lval_pop(a, 0));
  lval_del(a);
  return x;
}

lval* builtin_len(lenv* e, lval* a) {
  LASSERT_NUM("len", a, 1);
  LASSERT_TYPE("len", a, 0, LVAL_QEXPR);

  long x = a->cell[0]->count;
  lval_del(a);
  return lval_num(x);
}

lval* builtin_init(lenv* e, lval* a) {
  LASSERT_NUM("init", a, 1);
  LASSERT_TYPE("init", a, 0, LVAL_QEXPR);
  LASSERT_NOT_EMPTY("init", a, 0);

  lval* x = lval_take(a, 0);
  lval_pop(x, x->count-1);
  return x;
}

lval* builtin_var(lenv* e, lval* a, char* func) {
  LASSERT_TYPE(func, a, 0, LVAL_QEXPR);

  lval* syms = a->cell[0];
  for (int i = 0; i < syms->count; i++) {
    LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
        "Function %s cannot define non-symbol, argument %i. Got %s, Expected %s.",
        func, i, ltype_name(syms->cell[i]->type), ltype_name(LVAL_SYM));
  }

  LASSERT(a, (syms->count == a->count-1),
      "Function %s passed too many arguments for symbols. Got %i, Expected %i.",
      func, syms->count, a->count-1);

  for (int i = 0; i < syms->count; i++) {
    if (strcmp(func, "def") == 0) { lenv_def(e, syms->cell[i], a->cell[i+1]); }
    if (strcmp(func, "=") == 0)   { lenv_put(e, syms->cell[i], a->cell[i+1]); }
  }

  lval_del(a);
  return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a) { return builtin_var(e, a, "def"); }
lval* builtin_put(lenv* e, lval* a) { return builtin_var(e, a, "="); }

lval* builtin_env(lenv* e, lval* a) {
  lenv_iter(e);
  return lval_sexpr();
}

lval* builtin_lambda(lenv* e, lval* a) {
  LASSERT_NUM("\\", a, 2);
  LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
  LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

  // Check that first q-expr contains only symbols
  for (int i = 0; i < a->cell[0]->count; i++) {
    LASSERT(a, a->cell[0]->cell[i]->type == LVAL_SYM,
        "Cannot define non-symbol. Got %s, Expected %s.",
        ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
  }

  //Pop first two args and pass them to lval_lambda
  lval* formals = lval_pop(a, 0);
  lval* body = lval_pop(a, 0);
  lval_del(a);

  return lval_lambda(formals, body);
}


lval* builtin_gt(lenv*e, lval* a) { return builtin_ord(e, a, ">"); }
lval* builtin_lt(lenv*e, lval* a) { return builtin_ord(e, a, "<"); }
lval* builtin_ge(lenv*e, lval* a) { return builtin_ord(e, a, ">="); }
lval* builtin_le(lenv*e, lval* a) { return builtin_ord(e, a, "<="); }

lval* builtin_ord(lenv* e, lval* a, char* op) {
  LASSERT_NUM(op, a, 2);
  LASSERT_TYPE(op, a, 0, LVAL_NUM);
  LASSERT_TYPE(op, a, 1, LVAL_NUM);

  int r;
  if (strcmp(op, ">") == 0) { r = a->cell[0]->num > a->cell[1]->num; }
  if (strcmp(op, "<") == 0) { r = a->cell[0]->num < a->cell[1]->num; }
  if (strcmp(op, ">=") == 0) { r = a->cell[0]->num >= a->cell[1]->num; }
  if (strcmp(op, "<=") == 0) { r = a->cell[0]->num <= a->cell[1]->num; }

  lval_del(a);
  return lval_num(r);
}

lval* builtin_eq(lenv* e, lval* a) { return builtin_cmp(e, a, "=="); }
lval* builtin_ne(lenv* e, lval* a) { return builtin_cmp(e, a, "!="); }

lval* builtin_cmp(lenv* e, lval* a, char* op) {
  LASSERT_NUM(op, a, 2);
  int r;
  if (strcmp(op, "==") == 0) { r = lval_eq(a->cell[0], a->cell[1]); }
  if (strcmp(op, "!=") == 0) { r = !lval_eq(a->cell[0], a->cell[1]); }
  lval_del(a);
  return lval_num(r);
}

lval* builtin_if(lenv* e, lval* a) {
  LASSERT_NUM("if", a, 3);
  LASSERT_TYPE("if", a, 0, LVAL_NUM);
  LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
  LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

  //Make both expressions evaluatable
  lval* x;
  a->cell[1]->type = LVAL_SEXPR;
  a->cell[2]->type = LVAL_SEXPR;

  if (a->cell[0]->num) {
    x = lval_eval(e, lval_pop(a, 1));
  } else {
    x = lval_eval(e, lval_pop(a, 2));
  }
  lval_del(a);
  return x;
}

lval* builtin_and(lenv* e, lval* a) { return builtin_logic(e, a, "&&"); }
lval* builtin_or(lenv* e, lval* a) { return builtin_logic(e, a, "||"); }

lval* builtin_not(lenv* e, lval* a) {
  LASSERT_NUM("!", a, 1);
  LASSERT_TYPE("!", a, 0, LVAL_NUM);

  int r = !a->cell[0]->num;
  lval_del(a);
  return lval_num(r);
}

lval* builtin_logic(lenv* e, lval* a, char* op) {
  LASSERT_NUM(op, a, 2);
  LASSERT_TYPE(op, a, 0, LVAL_NUM);
  LASSERT_TYPE(op, a, 1, LVAL_NUM);

  int r;
  if (strcmp(op, "&&") == 0) { r = a->cell[0]->num && a->cell[1]->num; }
  if (strcmp(op, "||") == 0) { r = a->cell[0]->num || a->cell[1]->num; }
  lval_del(a);
  return lval_num(r);
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

lval* builtin_load(lenv* e, lval* a) {
  LASSERT_NUM("load", a, 1);
  LASSERT_TYPE("load", a, 0, LVAL_STR);

  //Parse file given by string as filename
  mpc_result_t r;
  if (mpc_parse_contents(a->cell[0]->str, blisp, &r)) {
    //Read Contents
    lval* expr = lval_read(r.output);
    mpc_ast_delete(r.output);

    //Evaluate expressions on stack
    while (expr->count) {
      lval* x = lval_eval(e, lval_pop(expr, 0));
      if (x->type == LVAL_ERR) {
        lval_println(x);
      }
      lval_del(x);
    }

    //Delete expression and args
    lval_del(expr);
    lval_del(a);

    //Return empty list
    return lval_sexpr();

  } else {
    //Get error message as string and return as lval_err
    char* err_msg = mpc_err_string(r.error);
    mpc_err_delete(r.error);

    lval* err = lval_err("Could not load Library %s", err_msg);
    free(err_msg);
    lval_del(a);

    //Cleanup and return error
    return err;
  }
}

lval* builtin_print(lenv* e, lval* a) {
  //Print each argument followed by a space
  for (int i = 0; i < a->count; i++) {
    lval_print(a->cell[i]); putchar(' ');
  }

  //Print a newline and delete args
  putchar('\n');
  lval_del(a);

  return lval_sexpr();
}

lval* builtin_error(lenv* e, lval* a) {
  LASSERT_NUM("error", a, 1);
  LASSERT_TYPE("error", a, 0, LVAL_STR);

  //Build error from first arg
  lval* err = lval_err(a->cell[0]->str);

  //clean up, return
  lval_del(a);
  return err;
}

