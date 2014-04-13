#include <stdio.h>
#include <math.h>
#include "lval.h"

// Create long value
lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

lval lval_dec(double x) {
  lval v;
  v.type = LVAL_DEC;
  v.dec = x;
  return v;
}

// Create error value
lval lval_err(lerr_t x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

/* lval arithmetic
 * Ugly typematching logic
 */

lval lval_add(lval x, lval y) {
  if (x.type == LVAL_DEC && y.type == LVAL_NUM) {
    return lval_dec(x.dec + y.num);
  } else if (x.type == LVAL_NUM && y.type == LVAL_DEC) {
    return lval_dec(x.num + y.dec);
  } else if (x.type == LVAL_DEC && y.type == LVAL_DEC) {
    return lval_dec(x.dec + y.dec);
  } else {
    return lval_num(x.num + y.num);
  }
}

lval lval_sub(lval x, lval y) {
  if (x.type == LVAL_DEC && y.type == LVAL_NUM) {
    return lval_dec(x.dec - y.num);
  } else if (x.type == LVAL_NUM && y.type == LVAL_DEC) {
    return lval_dec(x.num - y.dec);
  } else if (x.type == LVAL_DEC && y.type == LVAL_DEC) {
    return lval_dec(x.dec - y.dec);
  } else {
    return lval_num(x.num - y.num);
  }
}

lval lval_mul(lval x, lval y) {
  if (x.type == LVAL_DEC && y.type == LVAL_NUM) {
    return lval_dec(x.dec * y.num);
  } else if (x.type == LVAL_NUM && y.type == LVAL_DEC) {
    return lval_dec(x.num * y.dec);
  } else if (x.type == LVAL_DEC && y.type == LVAL_DEC) {
    return lval_dec(x.dec * y.dec);
  } else {
    return lval_num(x.num * y.num);
  }
}

lval lval_div(lval x, lval y) {
  if (x.type == LVAL_DEC && y.type == LVAL_NUM) {
    return lval_dec(x.dec / y.num);
  } else if (x.type == LVAL_NUM && y.type == LVAL_DEC) {
    return lval_dec(x.num / y.dec);
  } else if (x.type == LVAL_DEC && y.type == LVAL_DEC) {
    return lval_dec(x.dec / y.dec);
  } else {
    return lval_num(x.num / y.num);
  }
}

lval lval_pow(lval x, lval y) {
  if (x.type == LVAL_DEC && y.type == LVAL_NUM) {
    return lval_dec(pow(x.dec, y.num));
  } else if (x.type == LVAL_NUM && y.type == LVAL_DEC) {
    return lval_dec(pow(x.num, y.dec));
  } else if (x.type == LVAL_DEC && y.type == LVAL_DEC) {
    return lval_dec(pow(x.dec, y.dec));
  } else {
    return lval_num(pow(x.num, y.num));
  }
}

lval lval_min(lval x, lval y) {
  if (x.type == LVAL_DEC && y.type == LVAL_NUM) {
    return x.dec < y.num ? x : y;
  } else if (x.type == LVAL_NUM && y.type == LVAL_DEC) {
    return x.num < y.dec ? x : y;
  } else if (x.type == LVAL_DEC && y.type == LVAL_DEC) {
    return x.dec < y.dec ? x : y;
  } else {
    return x.num < y.num ? x : y;
  }
}

lval lval_max(lval x, lval y) {
  if (x.type == LVAL_DEC && y.type == LVAL_NUM) {
    return x.dec > y.num ? x : y;
  } else if (x.type == LVAL_NUM && y.type == LVAL_DEC) {
    return x.num > y.dec ? x : y;
  } else if (x.type == LVAL_DEC && y.type == LVAL_DEC) {
    return x.dec > y.dec ? x : y;
  } else {
    return x.num > y.num ? x : y;
  }
}

// Print contents of lval
void lval_print(lval v) {
  switch (v.type) {
    case LVAL_NUM: printf("%li", v.num); break;
    case LVAL_DEC: printf("%f", v.dec); break;
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
