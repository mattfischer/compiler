#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H

#include <vector>
#include "Front/ParseNode.h"

namespace Front {
	class Type;

	struct SyntaxNode {
		enum NodeType {
			NodeTypeList,
			NodeTypeProcedureDef,
			NodeTypeVarDecl,
			NodeTypeReturn,
			NodeTypePrint,
			NodeTypeAssign,
			NodeTypeIf,
			NodeTypeWhile,
			NodeTypeCompare,
			NodeTypeArith,
			NodeTypeConstant,
			NodeTypeId,
			NodeTypeCall
		};

		enum NodeSubtype {
			NodeSubtypeNone,

			NodeSubtypeAdd,
			NodeSubtypeMultiply,

			NodeSubtypeEqual,
			NodeSubtypeNequal
		};

		SyntaxNode(NodeType _nodeType, NodeSubtype _nodeSubtype = NodeSubtypeNone) : nodeType(_nodeType), nodeSubtype(_nodeSubtype) {}
		SyntaxNode() {}

		NodeType nodeType;
		NodeSubtype nodeSubtype;
		int line;
		std::vector<SyntaxNode*> children;
		Type *type;
		union {
			int _int;
			char *_id;
		} lexVal;

		static SyntaxNode *fromParseTree(ParseNode *tree);
	};
}

#endif