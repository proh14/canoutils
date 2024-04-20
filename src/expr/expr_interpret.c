#include "expr.h"

static int visit_binop(ast *node) {
  if (node->binop.prev == NULL || node->binop.next == NULL)
    return 0;
  switch (node->binop.tok->typ) {
  case TOK_ADD:
    return visit_generic(node->binop.prev) + visit_generic(node->binop.next);
  case TOK_SUB:
    return visit_generic(node->binop.prev) - visit_generic(node->binop.next);
  case TOK_MUL:
    return visit_generic(node->binop.prev) * visit_generic(node->binop.next);
  case TOK_DIV:
    return visit_generic(node->binop.prev) / visit_generic(node->binop.next);
  default:
    return 0;
  }
}

static int visit_num(ast *node) { return node->num.val; }

int visit_generic(ast *node) {
  if (node == NULL)
    return -1;
  switch (node->any.typ) {
  case AST_BINOP:
    return visit_binop(node);
  case AST_NUM:
    return visit_num(node);
  default:
    return -1;
  }
  return -1;
}
