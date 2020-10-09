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

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);

typedef enum
{
	ND_ADD,			// +
	ND_SUB, 		// -
	ND_MUL, 		// *
	ND_DIV,			// /
	ND_NEG,			// - num
	ND_EQ,			// ==
	ND_NE,			// !=
	ND_LT,			// <
	ND_LE,			// <=
	ND_NUM, 		// Int
	ND_EXPR_STMT,	//Expression stat
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *next;
	Node *lhs; // Left-hand side
	Node *rhs; // Right-hand side
	int val;
};

Node *parse(Token *tok);

void codeGeneration(Node *node);
