#include "compiler.h"

static char *current_input;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		error("%s: invalid number of args", argv[0]);
	}

	current_input = argv[1];
	Token *tok = tokenize(current_input);
	Node *node = parse(tok);
	codeGeneration(node);

	return 0;
}


