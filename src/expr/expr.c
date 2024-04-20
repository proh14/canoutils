#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"

#define NAME "expr (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Yohann Boniface (Sigmanificient)"

#include "version_info.h"

static void tree_free(ast *root) {
  if (root->any.typ == AST_UNARY)
    tree_free(root->unary.next);
  if (root->any.typ == AST_BINOP) {
    tree_free(root->binop.prev);
    tree_free(root->binop.next);
  }
  free(root);
}

static bool expr_run(char **argv) {
  lexer lex = {.argv = argv, 0};
  parser p = {.tok = lex_get_next_token(&lex), .lx = &lex};
  ast *root = parser_expr(&p);

  if (root == NULL) {
    fprintf(stderr, "Failed to create the AST");
    return EXIT_FAILURE;
  }
  free(lex.tokens);
  tree_free(root);
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
