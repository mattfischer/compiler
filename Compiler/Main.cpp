#include <stdio.h>

#include "ParseNode.h"
#include "Interpreter.h"

extern "C" ParseNode *parseLang(const char *filename);

int main(int arg, char *argv[])
{
	ParseNode *tree = parseLang("input.lang");
	Interpreter interpreter(tree);

	interpreter.run();

	return 0;
}