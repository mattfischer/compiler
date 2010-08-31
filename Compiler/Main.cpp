#include <stdio.h>

#include "ParseNode.h"
#include "SyntaxNode.h"
#include "TypeChecker.h"
#include "Interpreter.h"

extern "C" ParseNode *parseLang(const char *filename);

int main(int arg, char *argv[])
{
	ParseNode *parseTree = parseLang("input.lang");
	SyntaxNode *syntaxTree = SyntaxNode::fromParseTree(parseTree);
	TypeChecker typeChecker;
	typeChecker.check(syntaxTree);

	Interpreter interpreter(syntaxTree);

	interpreter.run();

	return 0;
}