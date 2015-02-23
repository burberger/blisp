/* Bob Urberger
 * 4/12/14
 * BLisp - Custom lisp implementation
 * REPL system
 */
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <editline/readline.h>
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "mpc.h"
#include "lval.h"
#include "builtin.h"

int main(int argc, char** argv) {
  // Create parser
  number  = mpc_new("number");
  symbol  = mpc_new("symbol");
  string  = mpc_new("string");
  comment = mpc_new("comment");
  sexpr   = mpc_new("sexpr");
  qexpr   = mpc_new("qexpr");
  expr    = mpc_new("expr");
  blisp   = mpc_new("blisp");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                  \
      number   : /-?[0-9]+/ ;                          \
      symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&%^|]+/ ; \
      string   : /\"(\\\\.|[^\"])*\"/ ;                \
      comment  : /;[^\\r\\n]*/ ;                       \
      sexpr    : '(' <expr>* ')' ;                     \
      qexpr    : '{' <expr>* '}' ;                     \
      expr     : <number>  | <symbol> | <string>       \
               | <comment> | <sexpr> | <qexpr> ;       \
      blisp    : /^/ <expr>* /$/ ;                     \
    ",
    number, symbol, string, comment, sexpr, qexpr, expr, blisp
  );

  // Build environment before running
  lenv* env = lenv_new();
  lenv_add_builtins(env);

  //Run interpreter
  if (argc == 1) {
    // Print version info
    puts("BLisp v0.0.1");
    puts("Press Ctrl+C to Exit");

    while (1) {
      char* input = readline("Blisp> ");

      //Skip input if blank
      if (strcmp(input, "") == 0) continue;

      add_history(input);

      //Parse input
      mpc_result_t r;
      if (mpc_parse("<stdin>", input, blisp, &r)) {
        lval* x = lval_eval(env, lval_read(r.output));
        lval_println(x);
        lval_del(x);
        mpc_ast_delete(r.output);
      } else {
        //Failed, print parser error
        mpc_err_print(r.error);
        mpc_err_delete(r.error);
      }

      free(input);
    }
  }

  //Process files on argument list
  if (argc >= 2) {
    //Loop over args, process contents, print any returned errors
    for (int i = 1; i < argc; i++) {
      lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
      lval* x = builtin_load(env, args);
      if (x->type == LVAL_ERR) {
        lval_println(x);
      }
      lval_del(x);
    }
  }

  //Cleanup parser before exiting
  mpc_cleanup(8, number, symbol, string, comment, sexpr, qexpr, expr, blisp);
  lenv_del(env);
  return 0;
}
