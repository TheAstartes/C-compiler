#include "compiler.h"

// Input string
static char *current_input;

// Reports an error and exit.
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// Reports an error location and exit.
static void verror_at(char *loc, char *fmt, va_list ap) {
  int pos = loc - current_input;
  fprintf(stderr, "%s\n", current_input);
  fprintf(stderr, "%*s", pos, ""); // print pos spaces.
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

// Consumes the current token if it matches `op`.
bool equal(Token *tok, char *op) {
  return strlen(op) == tok->len &&
         !strncmp(tok->loc, op, tok->len);
}

// Ensure that the current token is `op`.
Token *skip(Token *tok, char *op) {
  //printf("char is : %s",op);
  if (!equal(tok, op))
    error_tok(tok, "expected '%s'", op);
  return tok->next;
}

// Create a new token and add it as the next token of `cur`.
static Token *new_token(TokenKind kind, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

static bool is_iden(char c)
{
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_iden2(char c)
{
  return is_iden(c) || ('0' <= c && c <= '9');
}

static bool startswith(char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

// Tokenize `current_input` and returns new tokens.
Token *tokenize(char *p) {
  current_input = p;
  Token head = {};
  Token *cur = &head;

  while (*p) {
    // Skip whitespace characters.
    if (isspace(*p)) {
      p++;
      continue;
    }

    // Numeric literal
    if (isdigit(*p)) {
      cur = cur->next = new_token(TK_NUM, p, p);
      char *q = p;
      cur->val = strtoul(p, &p, 10);
      cur->len = p - q;
      //printf("cur->len is : %d \n", cur->len);
      continue;
    }

    if(is_iden(*p))
    {
      char *start = p;

      do
      {
        p++;
      } while(is_iden2(*p));

      cur = cur->next = new_token(TK_IDEN, start, p);

      continue;
    }

    // Multi-letter punctuators
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">=")) {
      cur = cur->next = new_token(TK_RESERVED, p, p + 2);
      p += 2;
      continue;
    }

    // Single-letter punctuators
    if (ispunct(*p)) {
      cur = cur->next = new_token(TK_RESERVED, p, p + 1);
      p++;
      continue;
    }

    error_at(p, "invalid token");
  }
  cur = cur->next = new_token(TK_EOF, p, p);
  return head.next;
}