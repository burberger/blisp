#include "mpc.h"
#include "uthash.h"

#ifndef LVAL_H
#define LVAL_H

// Forward declarations
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

// Enumeration of value types and error types
typedef enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR} ltype_t;

typedef lval*(*lbuiltin)(lenv*, lval*);

// Lisp value type
struct lval {
  ltype_t type;

  //numeric values
  union {
    long num;
    char* err;
    char* sym;
    lbuiltin fun;
  };

  int count;
  struct lval** cell;
};

struct lvar {
  char* sym;
  lval* val;
  UT_hash_handle hh;
};

struct lenv {
  struct lvar* vars;
};

//Environment functions
lenv* lenv_new(void);
void lenv_del(lenv* e);
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);

void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);

//Constructors
lval* lval_num(long x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_fun(lbuiltin func);
lval* lval_sexpr(void);
lval* lval_qexpr(void);

lval* lval_copy(lval* v);

//Destructor
void lval_del(lval* v);

//parser functions
lval* lval_add(lval* v, lval* x);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);

//Evaluator functions
lval* lval_pop(lval* v, int i);
lval* lval_take(lval* v, int i);
lval* lval_join(lval* x, lval* y);

lval* lval_eval_sexpr(lenv* e, lval* v);
lval* lval_eval(lenv* e, lval* v);

//Pretty printing objects
void lval_expr_print(lval* v, char first, char last);
void lval_print(lval* v);
void lval_println(lval* v);

#endif
