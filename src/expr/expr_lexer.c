#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"

static token_type CHAR_TOKENS[UCHAR_MAX] = {
    [(uchar)'+'] = TOK_ADD,
    [(uchar)'-'] = TOK_SUB,
    [(uchar)'*'] = TOK_MUL,
    [(uchar)'/'] = TOK_DIV,
};

static char const *TOKEN_REPR[] = {
    [TOK_UKN] = "???", [TOK_INT] = "int", [TOK_ADD] = "add", [TOK_SUB] = "sub",
    [TOK_MUL] = "mul", [TOK_DIV] = "div", [TOK_EOF] = "eof", [TOK_WIP] = "wip",
};

static token *get_new_token(lexer *lx) {
  size_t new_capacity;
  token *new;

  if (lx->token_count < lx->token_capacity)
    return &lx->tokens[lx->token_count++];

  new_capacity = (lx->token_capacity == 0) ? FIRST_TOKEN_BATCH_SIZE
                                           : lx->token_capacity << 1;
  new = realloc(lx->tokens, new_capacity * sizeof *lx->tokens);
  if (new == NULL)
    return NULL;
  lx->tokens = new;
  lx->token_capacity = new_capacity;
  return get_new_token(lx);
}

static inline token *mk_token(token *tokp, token_type typ, char *val,
                              size_t len) {
  *tokp = (token){.typ = typ, .val = val, .len = len};
  printf("T[%s]\n", TOKEN_REPR[tokp->typ]);
  return tokp;
}

token *lex_get_next_token(lexer *lx) {
  token *tokp = get_new_token(lx);
  char *arg = *lx->argv++;

  if (tokp == NULL)
    return NULL;
  if (arg == NULL)
    return mk_token(tokp, TOK_EOF, NULL, 0);

  tokp->typ = CHAR_TOKENS[(uchar)*arg];
  if (tokp->typ != TOK_UKN)
    return mk_token(tokp, tokp->typ, arg, strlen(arg));

  for (size_t i = 0; arg[i] != '\0'; i++)
    if (!isdigit(arg[i]))
      goto not_digit;

  return mk_token(tokp, TOK_INT, arg, strlen(arg));
not_digit:
  return mk_token(tokp, TOK_UKN, arg, strlen(arg));
}
