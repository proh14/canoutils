#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"

#define NAME "expr (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Yohann Boniface (Sigmanificient)"

#include "version_info.h"

static char const *TOKEN_REPR[] = {
    [TOK_UKN] = "???", [TOK_INT] = "int", [TOK_ADD] = "add", [TOK_SUB] = "sub",
    [TOK_MUL] = "mul", [TOK_DIV] = "div", [TOK_EOF] = "eof", [TOK_WIP] = "wip",
};

static bool expr_run(char **argv) {
  token *tokp;
  lexer lex = {.argv = argv};

  do {
    tokp = expr_lex_get_next_token(&lex);
    if (tokp == NULL) {
      fprintf(stderr, "Failed to tokeinize: %s\n", strerror(errno));
      free(lex.tokens);
      return false;
    }
    printf("T[%s](%.*s)\n", TOKEN_REPR[tokp->typ], (int)tokp->len, tokp->val);
  } while (tokp->typ != TOK_EOF);
  free(lex.tokens);
  return true;
}

int main(int argc, char **argv) {
  for (int i = 0; argv[i] != NULL; i++)
    if (!strcmp(argv[i], "--version")) {
      print_version();
      return EXIT_SUCCESS;
    }
  if (argc < 2) {
    fprintf(stderr, "expr: missing operand\n");
    return EXIT_FAILURE;
  }
  return expr_run(&argv[1]) ? EXIT_SUCCESS : EXIT_FAILURE;
}
