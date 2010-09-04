#include <stdio.h>

#include "Type.h"
#include "ParseNode.h"
#include "SyntaxNode.h"
#include "TypeChecker.h"
#include "Interpreter.h"
#include "IRGenerator.h"

extern "C" ParseNode *parseLang(const char *filename);

int main(int arg, char *argv[])
{
	Type::init();

	ParseNode *parseTree = parseLang("input.lang");
	SyntaxNode *syntaxTree = SyntaxNode::fromParseTree(parseTree);
	TypeChecker typeChecker;
	bool result = typeChecker.check(syntaxTree);

	if(result) {
		/*Interpreter interpreter(syntaxTree);
		interpreter.run();*/

		IRGenerator generator(syntaxTree);
		const IRGenerator::List &list = generator.generate();
		list.print();
		printf("\n");
		list.printGraph();
	}

	return 0;
}