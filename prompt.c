/* Bob Urberger
 * 4/12/14
 * BLisp - Custom lisp implementation
 * REPL system
 */
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"
#include "lval.h"

int main(int argc, char** argv) {
  // Create parser
  mpc_parser_t* number   = mpc_new("number");
  mpc_parser_t* symbol   = mpc_new("symbol");
  mpc_parser_t* sexpr    = mpc_new("sexpr");
  mpc_parser_t* qexpr    = mpc_new("qexpr");
  mpc_parser_t* expr     = mpc_new("expr");
  mpc_parser_t* blisp    = mpc_new("blisp");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                      \
      number   : /-?[0-9]+/ ;                              \
      symbol   : '+' | '-' | '*' | '/' | '%' | '^' ;       \
      sexpr    : '(' <expr>* ')' ;                         \
      qexpr    : '{' <expr>* '}' ;                         \
      expr     : <number> | <symbol> | <sexpr> | <qexpr> ; \
      blisp    : /^/ <expr>* /$/ ;                         \
    ",
    number, symbol, sexpr, qexpr, expr, blisp
  );

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
      /*mpc_ast_print(r.output);*/
      lval* x = lval_eval(lval_read(r.output));
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
  mpc_cleanup(6, number, symbol, sexpr, qexpr, expr, blisp);
  return 0;
}
