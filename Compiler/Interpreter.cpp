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
			{
				int result;

				evaluateExpression(node->children[0]);
				pop(&result, sizeof(result));

				printf("%i\n", result);
				break;
			}
	}
}

void Interpreter::evaluateExpression(SyntaxNode *node)
{
	switch(node->nodeType) {
		case SyntaxNode::NodeTypeConstant:
			push(&node->lexVal._int, sizeof(int));
			break;

		case SyntaxNode::NodeTypeAdd:
			{
				int a, b, c;
				evaluateExpression(node->children[0]);
				pop(&a, sizeof(a));

				evaluateExpression(node->children[1]);
				pop(&b, sizeof(b));

				c = a + b;
				push(&c, sizeof(c));
				break;
			}

		case SyntaxNode::NodeTypeMultiply:
			{
				int a, b, c;
				evaluateExpression(node->children[0]);
				pop(&a, sizeof(a));

				evaluateExpression(node->children[1]);
				pop(&b, sizeof(b));

				c = a * b;
				push(&c, sizeof(c));
				break;
			}
	}
}

void Interpreter::push(void *data, int size)
{
	char *c = (char*)data;

	for(int i=0; i<size; i++) {
		mStack.push_back(c[i]);
	}
}

void Interpreter::pop(void *data, int size)
{
	char *c = (char*)data;

	for(int i=size-1; i>=0; i--) {
		c[i] = mStack.back();
		mStack.pop_back();
	}
}