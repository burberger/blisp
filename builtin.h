#include "lval.h"

#ifndef BUILTIN_H
#define BUILTIN_H

lval* builtin_head(lval* a);
lval* builtin_tail(lval* a);
lval* builtin_list(lval* a);
lval* builtin_eval(lval* a);
lval* builtin_join(lval* a);
lval* builtin_cons(lval* a);
lval* builtin_len(lval* a);
lval* builtin_init(lval* a);

lval* builtin(lval* a, char* func);
lval* builtin_op(lval* a, char* op);

#endif
