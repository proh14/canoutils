#ifndef EXPR_H
#define EXPR_H

#include <stddef.h>

#define FIRST_TOKEN_BATCH_SIZE 64

typedef unsigned char uchar;

typedef enum {
  TOK_UKN = 0,
  TOK_INT,
  TOK_ADD,
  TOK_SUB,
  TOK_MUL,
  TOK_DIV,
  TOK_EOF,
  TOK_WIP,
} token_type;

typedef struct {
  token_type typ;
  char *val;
  size_t len;
} token;

typedef struct {
  char **argv;
  char prev;
  token *tokens;
  size_t token_count;
  size_t token_capacity;
} lexer;

typedef struct {
  token *tok;
  lexer *lx;
} parser;

typedef enum {
  AST_ANY,
  AST_UNARY,
  AST_BINOP,
  AST_NUM,
  AST_COUNT,
} ast_type;

union ast_;
typedef union ast_ {
  struct {
    token *tok;
    ast_type typ;
  } any;

  struct {
    token *tok;
    ast_type typ;
    union ast_ *next;
  } unary;

  struct {
    token *tok;
    ast_type typ;
    union ast_ *next;
    union ast_ *prev;
  } binop;

  struct {
    token *tok;
    ast_type typ;
    int val;
  } num;
} ast;

token *lex_get_next_token(lexer *lx);
ast *parser_expr(parser *p);
int visit_generic(ast *node);

#endif
