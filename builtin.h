#include "lval.h"

#ifndef BUILTIN_H
#define BUILTIN_H

//Macros for error checking
#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#define LASSERT_TYPE(func, args, index, expect) \
  LASSERT(args, args->cell[index]->type == expect, \
      "Function '%s' passed incorrect type for argument %i. Got %s, Expected %s", \
      func, index, ltype_name(args->cell[index]->type), ltype_name(expect))

#define LASSERT_NUM(func, args, num) \
  LASSERT(args, args->count == num, \
      "Function %s passed incorrect number of arguments. Got %i, Expected %i.", \
      func, args->count, num)

#define LASSERT_NOT_EMPTY(func, args, index) \
  LASSERT(args, args->cell[index]->count != 0, \
      "Function %s passed {} for argument %i.", func, index)

//Utility functions
char* ltype_name(ltype_t type);

//List functions
lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);
lval* builtin_len (lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);

//Variable functions
lval* builtin_var(lenv* e, lval* a, char* func);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);
lval* builtin_env(lenv* e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);

//Control flow
lval* builtin_gt(lenv*e, lval* a);
lval* builtin_lt(lenv*e, lval* a);
lval* builtin_ge(lenv*e, lval* a);
lval* builtin_le(lenv*e, lval* a);
lval* builtin_ord(lenv* e, lval* a, char* op);

lval* builtin_eq(lenv* e, lval* a);
lval* builtin_ne(lenv* e, lval* a);
lval* builtin_cmp(lenv* e, lval* a, char* op);

//Boolean logic
lval* builtin_and(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);
lval* builtin_logic(lenv* e, lval* a, char* op);

lval* builtin_if(lenv* e, lval* a);

//Math functions
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);

lval* builtin_op(lenv* e, lval* a, char* op);

//String functions
lval* builtin_load(lenv* e, lval* a);
lval* builtin_print(lenv* e, lval* a);
lval* builtin_error(lenv* e, lval* a);

#endif
