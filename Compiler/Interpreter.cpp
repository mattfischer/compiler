#include "Interpreter.h"

#include <stdio.h>

Interpreter::Interpreter(ParseNode *tree)
{
	mTree = tree;
}

void Interpreter::run()
{
	evaluateStatementList(mTree);
}

void Interpreter::evaluateStatementList(ParseNode *node)
{
	for(int i=0; i<node->numChildren; i++) {
		evaluateStatement(node->children[i]);
	}
}

void Interpreter::evaluateStatement(ParseNode *node)
{
	switch(node->type) {
		case ParseNodePrint:
			int result = evaluateExpression(node->children[0]);
			printf("%i\n", result);
			break;
	}
}

int Interpreter::evaluateExpression(ParseNode *node)
{
	switch(node->type) {
		case ParseNodeInt:
			return node->lexVal._int;

		case ParseNodeAdd:
			{
				int a = evaluateExpression(node->children[0]);
				int b = evaluateExpression(node->children[1]);
				return a + b;
			}

		case ParseNodeMultiply:
			{
				int a = evaluateExpression(node->children[0]);
				int b = evaluateExpression(node->children[1]);
				return a * b;
			}

		default:
			return 0;
	}
}