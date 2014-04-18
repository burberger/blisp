#include "mpc.h"

#ifndef LVAL_H
#define LVAL_H

/* Enumeration of value types and error types */
typedef enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR} ltype_t;
typedef enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM} lerr_t;

/* Lisp value type */
typedef struct lval {
  ltype_t type;

  //numeric values
  union {
    long num;
    char* err;
    char* sym;
  };

  int count;
  struct lval** cell;
} lval;

lval* lval_num(long x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

void lval_del(lval* v);

lval* lval_add(lval* v, lval* x);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);

lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);

lval* lval_eval_sexpr(lval* v);
lval* lval_eval(lval* v);

lval* builtin_op(lval* a, char* op);

void lval_expr_print(lval* v, char first, char last);
void lval_print(lval* v);
void lval_println(lval* v);

#endif
