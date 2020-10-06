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

//Parser

typedef enum
{
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NEG,	// - num
	ND_EQ,	// ==
	ND_NE,	// !=
	ND_LT,	// <
	ND_LE,	// <=
	ND_NUM, // Integer
} NodeKind;

// AST node type

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *lhs; // Left-hand side
	Node *rhs; // Right-hand side
	int val;
};

static Node *new_node(NodeKind kind)
{
	Node *node = calloc(1, sizeof(Node));
	node-> kind = kind;

	return node;
}

static Node *new_node_neg(NodeKind kind, Node *expr)
{
	Node *node = new_node(kind);
	node->lhs = expr;

	return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node*rhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;

	return node;
}

static Node *new_num(int val)
{
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

static Node *expr(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);
static Node *neg(Token **rest, Token *tok);
static Node *equals(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);

// expr = mul ("+) mul | "-" mul)*

static Node *expr(Token **rest, Token *tok)
{
  return equals(rest, tok);
}

static Node *equals(Token **rest, Token *tok)
{
	Node *node = relational(&tok, tok);

	while(true)
	{
		if( equal(tok, "=="))
		{
			node = new_binary(ND_EQ, node, relational(&tok, tok->next));
			continue;
		}

		if( equal(tok, "!="))
		{
			node = new_binary(ND_NE, node, relational(&tok, tok->next));
			continue;
		}

		*rest = tok;
    	return node;
	}
}

static Node *relational(Token **rest, Token *tok)
{
	Node *node = add(&tok, tok);

	while(true)
	{
		if (equal(tok, "<")) {
      node = new_binary(ND_LT, node, add(&tok, tok->next));
      continue;
    }

    if (equal(tok, "<=")) {
      node = new_binary(ND_LE, node, add(&tok, tok->next));
      continue;
    }

    if (equal(tok, ">")) {
      node = new_binary(ND_LT, add(&tok, tok->next), node);
      continue;
    }

    if (equal(tok, ">=")) {
      node = new_binary(ND_LE, add(&tok, tok->next), node);
      continue;
    }

    *rest = tok;
    return node;
	}
}

static Node *add(Token **rest, Token *tok)
{
	Node *node = neg(&tok, tok);

	while(true)
	{
		if(equal(tok, "+"))
		{
			node = new_binary(ND_ADD, node, neg(&tok, tok->next));
			continue;
		}

		if(equal(tok, "-"))
		{
			node = new_binary(ND_SUB, node, neg(&tok, tok->next));
			continue;
		}

		*rest = tok;

		return node;
	}	
}

// mul = neg ("*" neg | "/" neg)*
static Node *mul(Token **rest, Token *tok)
{
  Node *node = primary(&tok, tok);

  while(true)
  {
  	if(equal(tok, "*"))
  	{
  		node = new_binary(ND_MUL, node, primary(&tok, tok->next));
  		continue;
  	}

  	if(equal(tok, "/"))
  	{
  		node = new_binary(ND_DIV, node, primary(&tok, tok->next));
  		continue;
  	}

  	*rest = tok;

  	return node;
  }
}

static Node *neg(Token **rest, Token *tok)
{
	if(equal(tok, "+"))
	{
		return neg(rest, tok->next);
	}

	if(equal(tok, "-"))
	{
		return new_node_neg(ND_NEG, neg(rest, tok->next));

	}

	return primary(rest, tok);
}

// primary = "(" expr ")" | num

static Node *primary(Token **rest, Token *tok)
{
	if(equal(tok, ")"))
	{
		Node *node = expr(&tok, tok->next);

		*rest = skip(tok, ")");

		return node;
	}

	 if(equal(tok, "("))
	 {
	 	Node *node = expr(&tok, tok->next);

	 	*rest = skip(tok, ")");

	 	return node;
	 }

	 if(tok->kind == TK_NUM)
	 {
	 	Node *node = new_num(tok->val);

	 	*rest = tok->next;

	 	return node;
	 }

	 error_tok(tok, "expected an expression");
}

//Code generation

static int depth;

static void push(void)
{
	printf("push %%rax\n");
	depth++;
}

static void pop(char *arg)
{
	printf("pop %s\n", arg);
	depth--;
}

static void gen_expr(Node *node)
{
	switch(node->kind)
	{
		case ND_NUM:
			printf("mov $%d, %%rax\n", node->val );
			return;
		case ND_NEG:
			gen_expr(node->lhs);
			printf("neg %%rax\n");
			return;
	}

	gen_expr(node->rhs);
	push();
	gen_expr(node->lhs);
	pop("%rdi");

	switch (node->kind)
	{
		case ND_ADD:
			printf("add %%rdi, %%rax\n");
			return;
		case ND_SUB:
			printf(" sub %%rdi, %%rax\n");
			return;
		case ND_MUL:
			printf(" umul %%rdi, %%rax\n");
			return;
		case ND_DIV:
			printf(" cqo\n");
			printf(" idiv %%rdi\n");
			return;
		case ND_EQ:
 		case ND_NE:
  		case ND_LT:
  		case ND_LE:
    		printf("  cmp %%rdi, %%rax\n");

    		if (node->kind == ND_EQ)
      			printf("  sete %%al\n");
    		else if (node->kind == ND_NE)
     			 printf("  setne %%al\n");
    		else if (node->kind == ND_LT)
     			 printf("  setl %%al\n");
    		else if (node->kind == ND_LE)
     			 printf("  setle %%al\n");

    	printf("  movzb %%al, %%rax\n");
    	return;
	}

	error("invalid expression");
}

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		error("%s: invalid number of args", argv[0]);
	}

	//Tokenize and parse.

	current_input = argv[1];
	Token *tok = tokenize();
	Node *node = expr(&tok, tok);

	if (tok->kind != TK_EOF)
		error_tok(tok, "extra token");

	printf(" .global main\n");
	printf("main:\n");

	gen_expr(node);
	printf("ret\n");

	assert(depth == 0);

	return 0;
}

