#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ParseNode.h"

class Interpreter
{
public:
	Interpreter(ParseNode *tree);

	void run();

private:
	ParseNode *mTree;

	void evaluateStatementList(ParseNode *node);
	void evaluateStatement(ParseNode *node);
	int evaluateExpression(ParseNode *node);
};

#endif