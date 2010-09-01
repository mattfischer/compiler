#include "SyntaxNode.h"

#include <stdlib.h>

enum LexValType {
	LexValInt,
	LexValId,
	LexValNone
};

struct {
	SyntaxNode::NodeType nodeType;
	LexValType	lexType;
} convertTable[] = {
	{ /* ParseNodeStatementList	*/	SyntaxNode::NodeTypeStatementList,		LexValNone	},
	{ /* ParseNodePrint			*/	SyntaxNode::NodeTypePrintStatement,		LexValNone	},
	{ /* ParseNodeVarDecl		*/	SyntaxNode::NodeTypeVarDecl,			LexValNone	},
	{ /* ParseNodeAssign		*/	SyntaxNode::NodeTypeAssign,				LexValNone	},
	{ /* ParseNodeMultiply		*/	SyntaxNode::NodeTypeMultiply,			LexValNone	},
	{ /* ParseNodeAdd			*/	SyntaxNode::NodeTypeAdd,				LexValNone	},
	{ /* ParseNodeInt			*/	SyntaxNode::NodeTypeConstant,			LexValInt	},
	{ /* ParseNodeId			*/	SyntaxNode::NodeTypeId,					LexValId	}
};

SyntaxNode *SyntaxNode::fromParseTree(ParseNode *tree)
{
	SyntaxNode *node = new SyntaxNode;
	node->numChildren = tree->numChildren;
	node->children = new SyntaxNode*[node->numChildren];

	node->nodeType = convertTable[tree->type].nodeType;
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