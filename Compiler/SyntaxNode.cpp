#include "SyntaxNode.h"

#include <stdlib.h>

enum LexValType {
	LexValInt,
	LexValId,
	LexValNone
};

struct {
	SyntaxNode::NodeType nodeType;
	SyntaxNode::NodeSubtype nodeSubType;
	LexValType	lexType;
} convertTable[] = {
	{ /* ParseNodeStatementList	*/	SyntaxNode::NodeTypeStatementList,		SyntaxNode::NodeSubtypeNone,		LexValNone	},
	{ /* ParseNodePrint			*/	SyntaxNode::NodeTypePrintStatement,		SyntaxNode::NodeSubtypeNone,		LexValNone	},
	{ /* ParseNodeVarDecl		*/	SyntaxNode::NodeTypeVarDecl,			SyntaxNode::NodeSubtypeNone,		LexValNone	},
	{ /* ParseNodeAssign		*/	SyntaxNode::NodeTypeAssign,				SyntaxNode::NodeSubtypeNone,		LexValNone	},
	{ /* ParseNodeIf			*/	SyntaxNode::NodeTypeIf,					SyntaxNode::NodeSubtypeNone,		LexValNone	},
	{ /* ParseNodeWhile			*/	SyntaxNode::NodeTypeWhile,				SyntaxNode::NodeSubtypeNone,		LexValNone	},
	{ /* ParseNodeEqual			*/	SyntaxNode::NodeTypeCompare,			SyntaxNode::NodeSubtypeEqual,		LexValNone	},
	{ /* ParseNodeNequal		*/	SyntaxNode::NodeTypeCompare,			SyntaxNode::NodeSubtypeNequal,		LexValNone	},
	{ /* ParseNodeAdd			*/	SyntaxNode::NodeTypeArith,				SyntaxNode::NodeSubtypeAdd,			LexValNone	},
	{ /* ParseNodeMultiply		*/	SyntaxNode::NodeTypeArith,				SyntaxNode::NodeSubtypeMultiply,	LexValNone	},
	{ /* ParseNodeInt			*/	SyntaxNode::NodeTypeConstant,			SyntaxNode::NodeSubtypeNone,		LexValInt	},
	{ /* ParseNodeId			*/	SyntaxNode::NodeTypeId,					SyntaxNode::NodeSubtypeNone,		LexValId	}
};

SyntaxNode *SyntaxNode::fromParseTree(ParseNode *tree)
{
	SyntaxNode *node = new SyntaxNode;
	node->line = tree->line;
	node->numChildren = tree->numChildren;
	node->children = new SyntaxNode*[node->numChildren];

	node->nodeType = convertTable[tree->type].nodeType;
	node->nodeSubtype = convertTable[tree->type].nodeSubType;
	node->type = NULL;

	switch(convertTable[tree->type].lexType) {
		case LexValInt:
			node->lexVal._int = tree->lexVal._int;
			node->type = TypeInt;
			break;

		case LexValId:
			node->lexVal._id = tree->lexVal._id;
			break;

		case LexValNone:
			break;
	}

	for(int i=0; i<tree->numChildren; i++) {
		node->children[i] = fromParseTree(tree->children[i]);
	}

	return node;
}