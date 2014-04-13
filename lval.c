#include <stdio.h>
#include "lval.h"

// Create long value
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

// Create error value
lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

// Print contents of lval
void lval_print(lval v) {
  switch (v.type) {
    case LVAL_NUM: printf("%li", v.num); break;
    case LVAL_ERR:
      if (v.err == LERR_DIV_ZERO) printf("Error: Divide by zero");
      if (v.err == LERR_BAD_OP) printf("Error: Invalid operator");
      if (v.err == LERR_BAD_NUM) printf("Error: Invalid number");
    break;
  }
}

void lval_println(lval v) {
  lval_print(v);
  putchar('\n');
}
