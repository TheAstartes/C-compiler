#include "compiler.h"

static Node *expr(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);
static Node *neg(Token **rest, Token *tok);
static Node *equals(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);


static Node *new_node(NodeKind kind)
{
	Node *node = calloc(1, sizeof(Node));
	node-> kind = kind;

	return node;
}


static Node *new_binary(NodeKind kind, Node *lhs, Node*rhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;

	return node;
}


static Node *new_node_neg(NodeKind kind, Node *expr)
{
	Node *node = new_node(kind);
	node->lhs = expr;

	return node;
}

static Node *new_num(int val) {
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

static Node *stmt(Token **rest, Token *tok)
{
	return expr_stmt(rest, tok);
}

static Node *expr_stmt(Token **rest, Token *tok) 
{
  Node *node = new_node_neg(ND_EXPR_STMT, expr(&tok, tok));
  *rest = skip(tok, ";");
  return node;
}

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
	Node *node = mul(&tok, tok);

	while(true)
	{
		if(equal(tok, "+"))
		{
			node = new_binary(ND_ADD, node, mul(&tok, tok->next));
			continue;
		}

		if(equal(tok, "-"))
		{
			node = new_binary(ND_SUB, node, mul(&tok, tok->next));
			continue;
		}

		*rest = tok;

		return node;
	}	
}

// mul = neg ("*" neg | "/" neg)*
static Node *mul(Token **rest, Token *tok)
{
  Node *node = neg(&tok, tok);

  while(true)
  {
  	if(equal(tok, "*"))
  	{
  		node = new_binary(ND_MUL, node, neg(&tok, tok->next));
  		continue;
  	}

  	if(equal(tok, "/"))
  	{
  		node = new_binary(ND_DIV, node, neg(&tok, tok->next));
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

Node *parse(Token *tok)
{
	Node head = {};
	Node *cur = &head;

	while(tok->kind != TK_EOF)
	{
		cur = cur->next = stmt(&tok, tok);
	}

	return head.next;
}