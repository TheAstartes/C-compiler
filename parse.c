#include "initiate.h"

static Node *expr(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);
static Node *neg(Token **rest, Token *tok);
static Node *equals(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);


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


Node *parse(Token *tok)
{
	Node *node = expr(&tok, tok);
	if(tok->kind != TK_EOF)
	{
		error_tok(tok, "extra token");
	}

	return node;
}