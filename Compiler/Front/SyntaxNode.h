#ifndef SYNTAX_NODE_H
#define SYNTAX_NODE_H

#include "Front/ParseNode.h"

namespace Front {
	class Type;

	struct SyntaxNode {
		enum NodeType {
			NodeTypeProcedureList,
			NodeTypeProcedure,
			NodeTypeStatementList,
			NodeTypePrintStatement,
			NodeTypeVarDecl,
			NodeTypeAssign,
			NodeTypeIf,
			NodeTypeWhile,
			NodeTypeCompare,
			NodeTypeArith,
			NodeTypeConstant,
			NodeTypeId,
			NodeTypeCall,
			NodeTypeReturn,
			NodeTypeArgDeclList,
			NodeTypeArgumentDecl,
			NodeTypeArgList
		};

		enum NodeSubtype {
			NodeSubtypeNone,

			NodeSubtypeAdd,
			NodeSubtypeMultiply,

			NodeSubtypeEqual,
			NodeSubtypeNequal
		};

		NodeType nodeType;
		NodeSubtype nodeSubtype;
		int line;
		int numChildren;
		SyntaxNode **children;
		Type *type;
		union {
			int _int;
			char *_id;
		} lexVal;

		static SyntaxNode *fromParseTree(ParseNode *tree);
	};
}

#endif