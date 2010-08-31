#include "Interpreter.h"

#include <stdio.h>

Interpreter::Interpreter(SyntaxNode *tree)
{
	mTree = tree;
}

void Interpreter::run()
{
	evaluateStatementList(mTree);
}

void Interpreter::evaluateStatementList(SyntaxNode *node)
{
	for(int i=0; i<node->numChildren; i++) {
		evaluateStatement(node->children[i]);
	}
}

void Interpreter::evaluateStatement(SyntaxNode *node)
{
	switch(node->nodeType) {
		case SyntaxNode::NodeTypePrintStatement:
			int result = evaluateExpression(node->children[0]);
			printf("%i\n", result);
			break;
	}
}

int Interpreter::evaluateExpression(SyntaxNode *node)
{
	switch(node->nodeType) {
		case SyntaxNode::NodeTypeConstant:
			return node->lexVal._int;

		case SyntaxNode::NodeTypeAdd:
			{
				int a = evaluateExpression(node->children[0]);
				int b = evaluateExpression(node->children[1]);
				return a + b;
			}

		case SyntaxNode::NodeTypeMultiply:
			{
				int a = evaluateExpression(node->children[0]);
				int b = evaluateExpression(node->children[1]);
				return a * b;
			}

		default:
			return 0;
	}
}