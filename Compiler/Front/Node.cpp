#include "Front/Node.h"

const char *typeNames[] = {
	/* NodeTypeList         */ "list",
	/* NodeTypeProcedureDef */ "procedure",
	/* NodeTypeVarDecl      */ "declare",
	/* NodeTypeReturn       */ "return",
	/* NodeTypePrint        */ "print",
	/* NodeTypeAssign       */ "assign",
	/* NodeTypeIf           */ "if",
	/* NodeTypeWhile        */ "while",
	/* NodeTypeCompare      */ "compare",
	/* NodeTypeArith        */ "arithmetic",
	/* NodeTypeConstant     */ "constant",
	/* NodeTypeId           */ "identifier",
	/* NodeTypeCall         */ "call",
	/* NodeTypeArray        */ "array"
};

const char *subtypeNames[] = {
	/* NodeSubtypeNone     */ "none",
	/* NodeSubtypeAdd      */ "add",
	/* NodeSubtypeMultiply */ "multiply",
	/* NodeSubtypeEqual    */ "equal",
	/* NodeSubtypeNequal   */ "not-equal"
};

namespace Front {
	std::ostream &operator<<(std::ostream &o, Node *node)
	{
		o << typeNames[node->nodeType];
		if(node->nodeSubtype != Node::NodeSubtypeNone) {
			o << " | " << subtypeNames[node->nodeSubtype];
		}

		switch(node->nodeType) {
			case Node::NodeTypeConstant:
				o << " : " << node->lexVal.i;
				break;
			case Node::NodeTypeProcedureDef:
			case Node::NodeTypeVarDecl:
			case Node::NodeTypeId:
				o << " : \'" << node->lexVal.s << "\'";
				break;
		}

		if(node->type && !Type::equals(node->type, TypeNone)) {
			o << " (" << node->type->name << ")";
		}

		return o;
	}
}