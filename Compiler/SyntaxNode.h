#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H

#include "ParseNode.h"
#include "Type.h"

struct SyntaxNode {
	enum NodeType {
		NodeTypeStatementList,
		NodeTypePrintStatement,
		NodeTypeVarDecl,
		NodeTypeAssign,
		NodeTypeAdd,
		NodeTypeMultiply,
		NodeTypeConstant,
		NodeTypeId
	};

	NodeType nodeType;
	int numChildren;
	SyntaxNode **children;
	Type *type;
	union {
		int _int;
		char *_id;
	} lexVal;

	static SyntaxNode *fromParseTree(ParseNode *tree);
};
#endif