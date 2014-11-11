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

int main(int argc, char** argv) {
  // Create parser
  mpc_parser_t* number  = mpc_new("number");
  mpc_parser_t* symbol  = mpc_new("symbol");
  mpc_parser_t* string  = mpc_new("string");
  mpc_parser_t* comment = mpc_new("comment");
  mpc_parser_t* sexpr   = mpc_new("sexpr");
  mpc_parser_t* qexpr   = mpc_new("qexpr");
  mpc_parser_t* expr    = mpc_new("expr");
  mpc_parser_t* blisp   = mpc_new("blisp");

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

  // Print version info
  puts("BLisp v0.0.1");
  puts("Press Ctrl+C to Exit");

  // Build environment before running
  lenv* env = lenv_new();
  lenv_add_builtins(env);

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

  //Cleanup parser before exiting
  mpc_cleanup(8, number, symbol, string, comment, sexpr, qexpr, expr, blisp);
  lenv_del(env);
  return 0;
}
