#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

typedef enum
{
	TK_RESERVED,	//Keyword / punctuator
	TK_NUM,		//Numeric
	TK_EOF,		//End of life marker
} TokenKind;

//Token type

typedef struct Token Token;

struct Token
{
	TokenKind kind;	//Token kind
	Token *next;
	int val;	//If kind is TK_NUM, value
	char *loc;	//Token location
	int len;	//Token length
};

// Input string

static char *current_input;

//Report Error

static void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//Report error location

static void verror_at(char *loc, char *fmt, va_list ap)
{
	int pos = loc - current_input;

	fprintf(stderr, "%s\n", current_input);
	fprintf(stderr, "%*s\n", pos, "" );
	fprintf(stderr, " ^ ");
	fprintf(stderr, fmt, ap );
	fprintf(stderr, "\n" );

	exit(1);
}

static void error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap ,fmt);

	verror_at(loc, fmt, ap);
}

static void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

// If `s` matched consume token

static bool equal(Token *tok, char*s)
{
		return strlen(s) == tok->len && !strncmp(tok->loc, s, tok->len);
}

//Ensure that the current token is `s`

static Token *skip(Token *tok, char *s)
{
	if (!equal(tok,s))
		error_tok(tok, "expected '%s'", s);
	return tok->next;
}

// Ensure token is TK_NUM / numeric


static int get_number(Token *tok) {
  if (tok->kind != TK_NUM)
    error_tok(tok, "expected a number");
  return tok->val;
}

// Create a new token & add it as the next token

static Token *new_token(TokenKind kind, char *start, char *end)
{
	Token *tok = calloc(1, sizeof(Token));

	tok->kind = kind;
	tok->loc = start;
	tok->len = end - start;

	return tok;
}

// Tokenize `p` return new tokens

static Token *tokenize(void)
{
	char *p = current_input;

	Token head = {};
	Token *cur = &head;


	while(*p)
	{
		if(isspace(*p))
		{
			continue;
		}
		if (isdigit(*p))
		{
			cur = cur->next = new_token(TK_NUM, p, p);
			char *q = p;
			cur->val = strtoul(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		if (*p == '+' || *p == '-')
		{
			cur = cur->next = new_token(TK_RESERVED, p, p+1);
			p++;
			continue;
		}
		
		error_at(p, "Invalid token");
	}
	
	cur = cur->next = new_token(TK_EOF, p, p);

	return head.next;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: invalid number of arguments", argv[0]);

	current_input = argv[1];
	Token *tok = tokenize();

	printf(" .global main\n");
	printf("main:\n");

	// The first token must be a number
	 printf("  mov $%d, %%rax\n", get_number(tok));

	tok = tok->next;

	// ... followed by `+` number or `-` number

	while(tok->kind != TK_EOF)
	{
		if(equal(tok, "+"))
		{
			printf("add $%d, %%rax\n", get_number(tok->next));
			tok = tok->next->next;
			continue;
		}
	tok = skip(tok, "-");
	printf(" sub $%d, %%rax\n", get_number(tok));

	tok = tok->next;

	}

	
}
