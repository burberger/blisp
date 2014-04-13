#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lval.h"

// Create numeric lval and return pointer
lval* lval_num(long x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

// Create numeric lval and return pointer
lval* lval_dec(double x) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_DEC;
  v->dec = x;
  return v;
}

// Create error lval and return pointer
lval* lval_err(char* m) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

// Create symbol lval and return pointer
lval* lval_sym(char* s) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

lval* lval_sexpr(void) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

// Properly free all memory allocated for an lval
void lval_del(lval* v) {
  switch (v->type) {
    // Do nothing special for numbers
    case LVAL_NUM: break;
    case LVAL_DEC: break;

    // Free character buffers storing commands
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;

    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++) {
        lval_del(v->cell[i]);
      }
      free(v->cell);
    break;
  }
}

/* lval arithmetic
 * Ugly typematching logic
 */

lval* lval_add(lval *x, lval *y) {
  if (x->type == LVAL_DEC && y->type == LVAL_NUM) {
    return lval_dec(x->dec + y->num);
  } else if (x->type == LVAL_NUM && y->type == LVAL_DEC) {
    return lval_dec(x->num + y->dec);
  } else if (x->type == LVAL_DEC && y->type == LVAL_DEC) {
    return lval_dec(x->dec + y->dec);
  } else {
    return lval_num(x->num + y->num);
  }
}

lval* lval_sub(lval *x, lval *y) {
  if (x->type == LVAL_DEC && y->type == LVAL_NUM) {
    return lval_dec(x->dec - y->num);
  } else if (x->type == LVAL_NUM && y->type == LVAL_DEC) {
    return lval_dec(x->num - y->dec);
  } else if (x->type == LVAL_DEC && y->type == LVAL_DEC) {
    return lval_dec(x->dec - y->dec);
  } else {
    return lval_num(x->num - y->num);
  }
}

lval* lval_mul(lval *x, lval *y) {
  if (x->type == LVAL_DEC && y->type == LVAL_NUM) {
    return lval_dec(x->dec * y->num);
  } else if (x->type == LVAL_NUM && y->type == LVAL_DEC) {
    return lval_dec(x->num * y->dec);
  } else if (x->type == LVAL_DEC && y->type == LVAL_DEC) {
    return lval_dec(x->dec * y->dec);
  } else {
    return lval_num(x->num * y->num);
  }
}

lval* lval_div(lval *x, lval *y) {
  if (x->type == LVAL_DEC && y->type == LVAL_NUM) {
    return lval_dec(x->dec / y->num);
  } else if (x->type == LVAL_NUM && y->type == LVAL_DEC) {
    return lval_dec(x->num / y->dec);
  } else if (x->type == LVAL_DEC && y->type == LVAL_DEC) {
    return lval_dec(x->dec / y->dec);
  } else {
    return lval_num(x->num / y->num);
  }
}

lval* lval_pow(lval *x, lval *y) {
  if (x->type == LVAL_DEC && y->type == LVAL_NUM) {
    return lval_dec(pow(x->dec, y->num));
  } else if (x->type == LVAL_NUM && y->type == LVAL_DEC) {
    return lval_dec(pow(x->num, y->dec));
  } else if (x->type == LVAL_DEC && y->type == LVAL_DEC) {
    return lval_dec(pow(x->dec, y->dec));
  } else {
    return lval_num(pow(x->num, y->num));
  }
}

lval* lval_min(lval *x, lval *y) {
  if (x->type == LVAL_DEC && y->type == LVAL_NUM) {
    return x->dec < y->num ? *x : *y;
  } else if (x->type == LVAL_NUM && y->type == LVAL_DEC) {
    return x->num < y->dec ? *x : *y;
  } else if (x->type == LVAL_DEC && y->type == LVAL_DEC) {
    return x->dec < y->dec ? *x : *y;
  } else {
    return x->num < y->num ? *x : *y;
  }
}

lval* lval_max(lval *x, lval *y) {
  if (x->type == LVAL_DEC && y->type == LVAL_NUM) {
    return x->dec > y->num ? *x : *y;
  } else if (x->type == LVAL_NUM && y->type == LVAL_DEC) {
    return x->num > y->dec ? *x : *y;
  } else if (x->type == LVAL_DEC && y->type == LVAL_DEC) {
    return x->dec > y->dec ? *x : *y;
  } else {
    return x->num > y->num ? *x : *y;
  }
}

// Print contents of lval
void lval_print(lval* v) {
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

void lval_println(lval* v) {
  lval_print(v);
  putchar('\n');
}
