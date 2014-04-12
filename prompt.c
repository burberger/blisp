/* Bob Urberger
 * 4/12/14
 * BLisp - Custom lisp implementation
 * REPL system
 */
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"

int main(int argc, char** argv) {
  /* Create parser */
  mpc_parser_t* number   = mpc_new("number");
  mpc_parser_t* operator = mpc_new("operator");
  mpc_parser_t* expr     = mpc_new("expr");
  mpc_parser_t* blisp    = mpc_new("blisp");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                    \
      number   : /-?[0-9]*.?[0-9]+/ | /-?[0-9]+/ ;       \
      operator : '+' | '-' | '*' | '/' ;                 \
      expr     : <number> | '(' <operator> <expr>+ ')' ; \
      blisp    : /^/ <operator> <expr>+ /$/ ;            \
    ",
    number, operator, expr, blisp
  );

  // Print version info
  puts("BLisp v0.0.1");
  puts("Press Ctrl+C to Exit");

  while (1) {
    char* input = readline("Blisp> ");
    add_history(input);

    //Parse input
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, blisp, &r)) {
      //Parsed, print AST
      mpc_ast_print(r.output);
      mpc_ast_delete(r.output);
    } else {
      //Failed, print parser error
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }

    free(input);
  }

  //Cleanup parser before exiting
  mpc_cleanup(4, number, operator, expr, blisp);
  return 0;
}
