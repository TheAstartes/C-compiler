#include "initiate.h"

static char *current_input;

 void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}


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

 void error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap ,fmt);

	verror_at(loc, fmt, ap);
}

 void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

// If `s` matched consume token

 bool equal(Token *tok, char*s)
{
		return strlen(s) == tok->len && !strncmp(tok->loc, s, tok->len);
}

//Ensure that the current token is `s`

 Token *skip(Token *tok, char *s)
{
	if (!equal(tok,s))
		error_tok(tok, "expected '%s'", s);
	return tok->next;
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

static bool starts_with(char *p, char *q)
{
	return strncmp(p, q, strlen(q)) == 0;
}

// Tokenize `p` return new tokens

 Token *tokenize(char *p)
{
	p = current_input;

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

		if( starts_with(p, "==") ||
			starts_with(p, "!=") ||
			starts_with(p, "<=") ||
			starts_with(p, ">="))
		{
			cur = cur->next = new_token(TK_RESERVED, p, p+2);
			p += 2;

			continue;
		}

		if(ispunct(*p))
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
