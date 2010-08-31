#include "SyntaxNode.h"

#include <stdlib.h>

enum LexValType {
	LexValInt,
	LexValNone
};

struct {
	SyntaxNode::NodeType nodeType;
	LexValType	lexType;
} convertTable[] = {
	{ /* ParseNodeStatementList	*/	SyntaxNode::NodeTypeStatementList,		LexValNone },
	{ /* ParseNodePrint			*/	SyntaxNode::NodeTypePrintStatement,		LexValNone },
	{ /* ParseNodeMultiply		*/	SyntaxNode::NodeTypeMultiply,			LexValNone },
	{ /* ParseNodeAdd			*/	SyntaxNode::NodeTypeAdd,				LexValNone },
	{ /* ParseNodeInt			*/	SyntaxNode::NodeTypeConstant,			LexValInt }
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

		case LexValNone:
			break;
	}

	for(int i=0; i<tree->numChildren; i++) {
		node->children[i] = fromParseTree(tree->children[i]);
	}

	return node;
}