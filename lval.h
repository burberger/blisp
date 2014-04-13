#ifndef LVAL_H
#define LVAL_H

/* Enumeration of value types and error types */
typedef enum {LVAL_NUM, LVAL_DEC, LVAL_ERR} ltype_t;
typedef enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM} lerr_t;

/* Lisp value type */
typedef struct {
  ltype_t type;
  union {
    long num;
    double dec;
    lerr_t err;
  };
} lval;

lval lval_num(long x);
lval lval_dec(double x);
lval lval_err(lerr_t x);

// Operators on lvals
lval lval_add(lval x, lval y);
lval lval_sub(lval x, lval y);
lval lval_mul(lval x, lval y);
lval lval_div(lval x, lval y);
lval lval_pow(lval x, lval y);
lval lval_min(lval x, lval y);
lval lval_max(lval x, lval y);

void lval_print(lval v);
void lval_println(lval v);

#endif
