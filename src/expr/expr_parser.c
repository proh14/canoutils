#include <stdbool.h>
#include <stdlib.h>

#include "expr.h"

static bool expr_parser_eat(parser *p, token_type tt) {
  if (p->tok->typ != tt)
    return false;
  p->tok = lex_get_next_token(p->lx);
  return true;
}

ast *parser_factor(parser *p) {
  ast *node = malloc(sizeof *node);
  token *tmp;

  if (p->tok == NULL)
    return NULL;
  if (p->tok->typ == TOK_INT) {
    tmp = p->tok;
    expr_parser_eat(p, TOK_INT);
    node->num.val = strtoll(tmp->val, NULL, 10);
    node->num.tok = p->tok;
    node->num.typ = AST_NUM;
    return node;
  }
  return NULL;
}

ast *parser_term(parser *p) {
  ast *node = parser_factor(p);
  ast *prev = node;
  token *tmp;

  if (p->tok == NULL)
    return NULL;
  while (p->tok->typ == TOK_MUL || p->tok->typ == TOK_DIV) {
    tmp = p->tok;
    if (p->tok->typ == TOK_MUL)
      expr_parser_eat(p, TOK_MUL);
    else if (p->tok->typ == TOK_DIV)
      expr_parser_eat(p, TOK_DIV);
    prev = node;
    node = malloc(sizeof *node);
    node->binop.prev = prev;
    node->binop.tok = tmp;
    node->binop.next = parser_factor(p);
    node->binop.typ = AST_BINOP;
    if (p->tok == NULL)
      return NULL;
  }
  return node;
}

ast *parser_expr(parser *p) {
  ast *node = parser_term(p);
  ast *prev = node;
  token *tmp;

  if (p->tok == NULL)
    return NULL;
  while (p->tok->typ == TOK_ADD || p->tok->typ == TOK_SUB) {
    tmp = p->tok;
    if (p->tok->typ == TOK_ADD)
      expr_parser_eat(p, TOK_ADD);
    else if (p->tok->typ == TOK_SUB)
      expr_parser_eat(p, TOK_SUB);
    prev = node;
    node = malloc(sizeof *node);
    node->binop.prev = prev;
    node->binop.tok = tmp;
    node->binop.next = parser_term(p);
    node->binop.typ = AST_BINOP;
    if (p->tok == NULL)
      return NULL;
  }
  return node;
}
