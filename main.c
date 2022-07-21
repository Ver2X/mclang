#include "chibicc.h"

// current token
Token * token;

// inputs
char *user_input;

int main(int argc, char ** argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "number of argument is error!\n");
		return 1;
	}


	// get first token
	user_input = argv[1];
	token = tokenize();
	Node * node = expr();


	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	gen(node);


	printf("  pop rax\n");
	printf("  ret\n");
	return 0;
}
