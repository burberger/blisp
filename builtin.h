#include "lval.h"

#ifndef BUILTIN_H
#define BUILTIN_H

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

//Language features
lval* builtin_var(lenv* e, lval* a, char* func);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);


//Math functions
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);

lval* builtin_op(lenv* e, lval* a, char* op);


#endif