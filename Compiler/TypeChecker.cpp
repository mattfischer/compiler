#include "TypeChecker.h"

#include <stdio.h>

bool TypeChecker::check(SyntaxNode *tree)
{
	for(int i=0; i<tree->numChildren; i++) {
		check(tree->children[i]);
	}

	switch(tree->nodeType) {
		case SyntaxNode::NodeTypeStatementList:
		case SyntaxNode::NodeTypePrintStatement:
			tree->type = TypeNone;
			break;

		case SyntaxNode::NodeTypeAdd:
		case SyntaxNode::NodeTypeMultiply:
			if(tree->children[0]->type != TypeInt ||
				tree->children[1]->type != TypeInt) {
					printf("Error: Type mismatch\n");
					return false;
			}
			tree->type = TypeInt;
	}

	return true;
}
					