#include "Front/Node.h"

#include "Front/Types.h"

const char *typeNames[] = {
	/* NodeTypeList         */ "list",
	/* NodeTypeProcedureDef */ "procedure",
	/* NodeTypeVarDecl      */ "declare",
	/* NodeTypeStructDef    */ "struct",
	/* NodeTypeClassDef     */ "class",
	/* NodeTypeReturn       */ "return",
	/* NodeTypePrint        */ "print",
	/* NodeTypeAssign       */ "assign",
	/* NodeTypeIf           */ "if",
	/* NodeTypeWhile        */ "while",
	/* NodeTypeFor          */ "for",
	/* NodeTypeCompare      */ "compare",
	/* NodeTypeArith        */ "arithmetic",
	/* NodeTypeConstant     */ "constant",
	/* NodeTypeId           */ "identifier",
	/* NodeTypeCall         */ "call",
	/* NodeTypeArray        */ "array",
	/* NodeTypeNew          */ "new",
	/* NodeTypeBreak        */ "break",
	/* NodeTypeContinue     */ "continue",
	/* NodeTypeMember       */ "member",
	/* NodeTypeCoerceString */ "coerce"
};

const char *subtypeNames[] = {
	/* NodeSubtypeNone             */ "none",
	/* NodeSubtypeAdd              */ "add",
	/* NodeSubtypeSubtract         */ "subtract",
	/* NodeSubtypeMultiply         */ "multiply",
	/* NodeSubtypeDivide           */ "divide",
	/* NodeSubtypeModulo           */ "modulo",
	/* NodeSubtypeIncrement        */ "increment",
	/* NodeSubtypeDecrement        */ "decrement",
	/* NodeSubtypeEqual            */ "equal",
	/* NodeSubtypeNequal           */ "not-equal",
	/* NodeSubtypeLessThan         */ "less-than",
	/* NodeSubtypeLessThanEqual    */ "less-than-equal",
	/* NodeSubtypeGreaterThan      */ "greater-than",
	/* NodeSubtypeGreaterThanEqual */ "greater-than-equal",
	/* NodeSubtypeOr               */ "or",
	/* NodeSubtypeAnd              */ "and"
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
				if(Type::equals(node->type, Types::intrinsic(Types::String))) {
					o << " : '" << node->lexVal.s << "'";
				} else {
					o << " : " << node->lexVal.i;
				}
				break;
			case Node::NodeTypeProcedureDef:
			case Node::NodeTypeVarDecl:
			case Node::NodeTypeId:
			case Node::NodeTypeMember:
				o << " : \'" << node->lexVal.s << "\'";
				break;
		}

		if(node->type && node->type->name != "void") {
			o << " (" << node->type->name << ")";
		}

		return o;
	}
}