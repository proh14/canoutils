#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"

#define NAME "expr (canoutils)"
#define VERSION "1.0.0"
#define AUTHOR "Yohann Boniface (Sigmanificient)"

#include "version_info.h"

void tree_free(ast *root) {
  if (root == NULL)
    return;
  if (root->any.typ == AST_UNARY)
    tree_free(root->unary.next);
  if (root->any.typ == AST_BINOP) {
    tree_free(root->binop.prev);
    tree_free(root->binop.next);
  }
  free(root);
}

static void print_depth(int depth) {
  for (int i = 1; i < depth; i++)
    printf("  ");
  if (depth)
    printf("- ");
}

static void traverse_ast(ast *root, int depth) {
  if (root->any.typ == AST_NUM) {
    print_depth(depth);
    printf("(int: %d)", root->num.val);
    return;
  }
  print_depth(depth);
  if (root->any.typ == AST_UNARY) {
    printf("([%s]\n", root->unary.tok->val);
    traverse_ast(root->unary.next, depth + 1);
  } else if (root->any.typ == AST_BINOP) {
    printf("([%s]\n", root->binop.tok->val);
    traverse_ast(root->binop.prev, depth + 1);
    printf("\n");
    traverse_ast(root->binop.next, depth + 1);
  }
  printf(")");
  if (depth == 0)
    printf("\n");
}

static bool expr_run(char **argv) {
  lexer lex = {.argv = argv, 0};
  parser p = {.tok = lex_get_next_token(&lex), .lx = &lex};
  ast *root = parser_expr(&p);

  if (root == NULL)
    return free(lex.tokens), EXIT_FAILURE;
  traverse_ast(root, 0);
  printf("%d\n", visit_generic(root));
  tree_free(root);
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
