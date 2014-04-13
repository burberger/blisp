/* Bob Urberger
 * 4/12/14
 * BLisp - Custom lisp implementation
 * REPL system
 */
#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

#include "mpc.h"

//Core set of operators
long eval_operator(long x, char* op, long y) {
  if (strcmp(op, "+") == 0) return x + y;
  if (strcmp(op, "-") == 0) return x - y;
  if (strcmp(op, "*") == 0) return x * y;
  if (strcmp(op, "/") == 0) return x / y;
  if (strcmp(op, "%") == 0) return x % y;
  if (strcmp(op, "^") == 0) return pow(x, y);

  if (strcmp(op, "min") == 0) return x < y ? x : y;
  if (strcmp(op, "max") == 0) return x > y ? x : y;

  return 0;
}

long eval(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  //Second child is always operator
  char* op = t->children[1]->contents;

  //store 3rd child in x
  long x = eval(t->children[2]);

  //Recursively evaluate operators in the tree
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_operator(x, op, eval(t->children[i]));
    i++;
  }

  //Support for negating single argument to - operator
  if (strcmp(op, "-") == 0 && i == 3) {
    return -x;
  }

  return x;
}

int main(int argc, char** argv) {
  /* Create parser */
  mpc_parser_t* number   = mpc_new("number");
  mpc_parser_t* function = mpc_new("function");
  mpc_parser_t* operator = mpc_new("operator");
  mpc_parser_t* expr     = mpc_new("expr");
  mpc_parser_t* blisp    = mpc_new("blisp");

  mpca_lang(MPC_LANG_DEFAULT,
    "                                                             \
      number   : /-?[0-9]*[.]?[0-9]+/ | /-?[0-9]+/ ;              \
      function : /[a-zA-Z0-9]+/ ;                                 \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | <function> ; \
      expr     : <number> | '(' <operator> <expr>+ ')' ;          \
      blisp    : /^/ <operator> <expr>+ /$/ ;                     \
    ",
    number, function, operator, expr, blisp
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
      //Evaluate expression tree and print result
      mpc_ast_print(r.output);
      long result = eval(r.output);
      printf("%li\n", result);
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
