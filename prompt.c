/* Bob Urberger
 * 4/12/14
 * BLisp - Custom lisp implementation
 * REPL system
 */
#include <stdio.h>

// Max input size of 2048
static char input[2048];

int main(int argc, char** argv) {
    // Print version info
    puts("BLisp v0.0.1");
    puts("Press Ctrl+C to Exit");

    while (1) {
        fputs("Blisp> ", stdout);
        fgets(input, 2048, stdin);
        printf("%s", input);
    }
}
