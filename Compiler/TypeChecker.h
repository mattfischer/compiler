#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "SyntaxNode.h"

class TypeChecker
{
public:
	bool check(SyntaxNode *tree);
};
#endif