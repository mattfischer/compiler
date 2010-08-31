#include <stdio.h>

#include "ParseNode.h"
#include "SyntaxNode.h"
#include "Interpreter.h"

extern "C" ParseNode *parseLang(const char *filename);

int main(int arg, char *argv[])
{
	ParseNode *parseNode = parseLang("input.lang");
	SyntaxNode *syntaxNode = SyntaxNode::fromParseTree(parseNode);

	Interpreter interpreter(syntaxNode);

	interpreter.run();

	return 0;
}