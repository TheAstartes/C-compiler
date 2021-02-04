#define _POSIX_C_SOURCE 200809L

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
	TK_IDEN,	//Identifier
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

typedef struct Obj Obj;

struct Obj
{
	Obj *next;
	char *name;
	int offset;
};

typedef struct Node Node;

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
	ND_ASSIGN,		//Assign  (a = 1 )
	ND_VAR,			// Variable
	ND_IF,			// If statements
	ND_BLOCK,		//Block of code
	ND_RETURN,		// Return keyword
	ND_FOR,			// For statement
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *next;
	Node *lhs; 			// Left-hand side
	Node *rhs; 			// Right-hand side
	Node *body;			// Body inside brackets
	Node *condition;	// Condition
	Node *then;			// 
	Node *els;			// else
	int val;			//name for variable
	Obj *var;
	Node *initiate;
	Node *increment;
};


typedef struct Function Function;

struct Function
{
	Node *body;
	Obj *objects;
	int stack_size;
};

Function *parse(Token *tok);

void codeGeneration(Function *pr);
