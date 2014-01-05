#ifndef FRONT_NODE_H
#define FRONT_NODE_H

#include <vector>

namespace Front {
	class Type;

	struct Node {
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

		NodeType nodeType;
		NodeSubtype nodeSubtype;
		int line;
		std::vector<Node*> children;
		Type *type;

		struct LexVal {
			int i;
			std::string s;
		};
		LexVal lexVal;
	};
}

#endif