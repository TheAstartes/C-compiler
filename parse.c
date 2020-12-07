#include "compiler.h"

static Node *expr(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);
static Node *neg(Token **rest, Token *tok);
static Node *equals(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *block_stmt(Token **rest, Token *tok);

Obj *objects;

static Obj *find_var(Token *tok)
{
  for (Obj *var = objects; var; var = var->next)
  {
    if(strlen(var->name) == tok->len && !strncmp(tok->loc, var->name, tok->len))
    {
      return var;
    }

    return NULL;
  }
}

static Node *new_node(NodeKind kind)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_neg_node(NodeKind kind, Node *expr)
{
  Node *node = new_node(kind);
  node->lhs = expr;
  return node;
}

static Node *new_num(int val)
{
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

static Node *new_var_node(Obj *var)
{
  Node *node = new_node(ND_VAR);
  node->var = var;
  return node;
}

static Obj *new_lvar(char *name)
{
  Obj *var = calloc(1, sizeof(Obj));
  var->name = name;
  var->next = objects;

  objects = var;
  return var;
}

static Node *stmt(Token **rest, Token *tok)
{
  if (equal(tok, "return"))
  {
  	Node *node = new_neg_node(ND_RETURN, expr(&tok, tok->next));
  	*rest = skip(tok, ";");
  	return node;
  }

  if(equal(tok, "if"))
  {
    Node *node = new_node(ND_IF);

    tok = skip(tok->next, "(");
    node->condition = expr(&tok, tok);
    tok = skip(tok, ")");

    node->then = stmt(&tok, tok);

    if(equal(tok, "else"))
    {
      node->els = stmt(&tok, tok->next);
    }

    *rest = tok;

    return node;
  }

  if(equal(tok, "{"))
  {
    return block_stmt(rest, tok);
  }

  return expr_stmt(rest, tok);
}


static Node *block_stmt(Token **rest, Token *tok)
{
  Node head = {};
  Node *cur = &head;

  while(!equal(tok, "}"))
  {
    cur = cur->next = stmt(&tok, tok);

    Node *node = new_node(ND_BLOCK);
    node->body = head.next;
    *rest = tok->next;

    return node;
  }
}

static Node *expr_stmt(Token **rest, Token *tok)
{
  if(equal(tok, ";"))
  {
    *rest = tok->next;

    return new_node(ND_BLOCK);
  }


  Node *node = new_neg_node(ND_EXPR_STMT, expr(&tok, tok));
  *rest = skip(tok, ";");
  return node;
}


static Node *expr(Token **rest, Token *tok)
{
  return assign(rest, tok);
}

static Node *assign(Token **rest, Token *tok)
{
	Node *node = equals(&tok, tok);
	if(equal(tok, "="))
	{
		node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next));
	}
	*rest = tok;
	return node;
}

static Node *equals(Token **rest, Token *tok)
{
  Node *node = relational(&tok, tok);

  while(true)
  {
    if (equal(tok, "=="))
    {
      node = new_binary(ND_EQ, node, relational(&tok, tok->next));
      continue;
    }

    if (equal(tok, "!="))
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

  for (;;) {
    if (equal(tok, "<"))
    {
      node = new_binary(ND_LT, node, add(&tok, tok->next));
      continue;
    }

    if (equal(tok, "<="))
    {
      node = new_binary(ND_LE, node, add(&tok, tok->next));
      continue;
    }

    if (equal(tok, ">"))
    {
      node = new_binary(ND_LT, add(&tok, tok->next), node);
      continue;
    }

    if (equal(tok, ">="))
    {
      node = new_binary(ND_LE, add(&tok, tok->next), node);
      continue;
    }

    *rest = tok;
    return node;
  }
}

// add = mul ("+" mul | "-" mul)*
static Node *add(Token **rest, Token *tok)
{
  Node *node = mul(&tok, tok);

  while(true)
  {
    if (equal(tok, "+"))
    {
      node = new_binary(ND_ADD, node, mul(&tok, tok->next));
      continue;
    }

    if (equal(tok, "-"))
    {
      node = new_binary(ND_SUB, node, mul(&tok, tok->next));
      continue;
    }

    *rest = tok;
    return node;
  }
}

static Node *mul(Token **rest, Token *tok)
{
  Node *node = neg(&tok, tok);

   while(true)
  {
    if (equal(tok, "*"))
    {
      node = new_binary(ND_MUL, node, neg(&tok, tok->next));
      continue;
    }

    if (equal(tok, "/"))
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
  if (equal(tok, "+"))
    return neg(rest, tok->next);

  if (equal(tok, "-"))
    return new_neg_node(ND_NEG, neg(rest, tok->next));

  return primary(rest, tok);
}

static Node *primary(Token **rest, Token *tok)
{
  if (equal(tok, "("))
  {
    Node *node = expr(&tok, tok->next);
    *rest = skip(tok, ")");
    return node;
  }

  if(tok->kind == TK_IDEN)
  {
    Obj *var = find_var(tok);
    if(!var)
    {
      var = new_lvar(strndup(tok->loc, tok->len));
    }
    *rest = tok->next;
    return new_var_node(var);
  }

  if (tok->kind == TK_NUM)
  {
    Node *node = new_num(tok->val);
    *rest = tok->next;
    return node;
  }

  error_tok(tok, "expected an expression");
}

Function *parse(Token *tok)
{
  tok = skip(tok, "{");
  
  Function *pr = calloc(1, sizeof(Function));

  pr->body = block_stmt(&tok, tok);
  pr->objects = objects;

  return pr;
}
