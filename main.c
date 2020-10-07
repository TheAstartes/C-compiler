#include "initiate.h"


int main(int argc, char **argv)
{
	if(argc != 2)
	{
		error("%s: invalid number of args", argv[0]);
	}

	
	Token *tok = tokenize(argv[1]);
	Node *node = parse(tok);

	codeGeneration(node);

	return 0;
}


