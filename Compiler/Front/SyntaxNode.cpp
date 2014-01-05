#include "SyntaxNode.h"

#include "Front/Type.h"

#include <stdlib.h>

namespace Front {
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
		{ /* ParseNodeList			*/  SyntaxNode::NodeTypeList,				SyntaxNode::NodeSubtypeNone,		LexValNone	},
		{ /* ParseNodeProcedureDef  */  SyntaxNode::NodeTypeProcedureDef,       SyntaxNode::NodeSubtypeNone,		LexValId	},
		{ /* ParseNodeVarDecl		*/	SyntaxNode::NodeTypeVarDecl,			SyntaxNode::NodeSubtypeNone,		LexValId	},
		{ /* ParseNodeReturn		*/	SyntaxNode::NodeTypeReturn,				SyntaxNode::NodeSubtypeNone,		LexValNone	},
		{ /* ParseNodePrint			*/	SyntaxNode::NodeTypePrint,				SyntaxNode::NodeSubtypeNone,		LexValNone	},
		{ /* ParseNodeAssign		*/	SyntaxNode::NodeTypeAssign,				SyntaxNode::NodeSubtypeNone,		LexValNone	},
		{ /* ParseNodeIf			*/	SyntaxNode::NodeTypeIf,					SyntaxNode::NodeSubtypeNone,		LexValNone	},
		{ /* ParseNodeWhile			*/	SyntaxNode::NodeTypeWhile,				SyntaxNode::NodeSubtypeNone,		LexValNone	},
		{ /* ParseNodeEqual			*/	SyntaxNode::NodeTypeCompare,			SyntaxNode::NodeSubtypeEqual,		LexValNone	},
		{ /* ParseNodeNequal		*/	SyntaxNode::NodeTypeCompare,			SyntaxNode::NodeSubtypeNequal,		LexValNone	},
		{ /* ParseNodeAdd			*/	SyntaxNode::NodeTypeArith,				SyntaxNode::NodeSubtypeAdd,			LexValNone	},
		{ /* ParseNodeMultiply		*/	SyntaxNode::NodeTypeArith,				SyntaxNode::NodeSubtypeMultiply,	LexValNone	},
		{ /* ParseNodeInt			*/	SyntaxNode::NodeTypeConstant,			SyntaxNode::NodeSubtypeNone,		LexValInt	},
		{ /* ParseNodeId			*/	SyntaxNode::NodeTypeId,					SyntaxNode::NodeSubtypeNone,		LexValId	},
		{ /* ParseNodeCall			*/	SyntaxNode::NodeTypeCall,				SyntaxNode::NodeSubtypeNone,		LexValNone	}
	};

	SyntaxNode *SyntaxNode::fromParseTree(ParseNode *tree)
	{
		SyntaxNode *node = new SyntaxNode;
		node->line = tree->line;
		node->children.reserve(tree->numChildren);

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
}
