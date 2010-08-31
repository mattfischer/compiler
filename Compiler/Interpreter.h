#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "SyntaxNode.h"

class Interpreter
{
public:
	Interpreter(SyntaxNode *tree);

	void run();

private:
	SyntaxNode *mTree;

	void evaluateStatementList(SyntaxNode *node);
	void evaluateStatement(SyntaxNode *node);
	int evaluateExpression(SyntaxNode *node);
};

#endif