#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "SyntaxNode.h"

#include <vector>

class Interpreter
{
public:
	Interpreter(SyntaxNode *tree);

	void run();

private:
	SyntaxNode *mTree;

	void evaluateStatementList(SyntaxNode *node);
	void evaluateStatement(SyntaxNode *node);
	void evaluateExpression(SyntaxNode *node);

	void push(void *data, int size);
	void pop(void *data, int size);

	std::vector<char> mStack;
};

#endif