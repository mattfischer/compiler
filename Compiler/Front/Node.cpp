#include "Front/Node.h"

#include "Front/Types.h"

const char *typeNames[] = {
	/* NodeTypeList         */ "list",
	/* NodeTypeProcedureDef */ "procedure",
	/* NodeTypeVarDecl      */ "declare",
	/* NodeTypeStructDef    */ "struct",
	/* NodeTypeClassDef     */ "class",
	/* NodeTypeClassMember  */ "member",
	/* NodeTypeReturn       */ "return",
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
	/* NodeTypeCoerce       */ "coerce",
	/* NodeTypeQualifier    */ "qualifier"
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
	/* NodeSubtypeAnd              */ "and",
	/* NodeSubtypeVirtual          */ "virtual",
	/* NodeSubtypeNative           */ "native"
};

namespace Front {
	std::ostream &operator<<(std::ostream &o, Node *node)
	{
		o << typeNames[(int)node->nodeType];
		if(node->nodeSubtype != Node::Subtype::None) {
			o << " | " << subtypeNames[node->nodeSubtype];
		}

		switch(node->nodeType) {
			case Node::Type::Constant:
				if(Type::equals(*node->type, *Types::intrinsic(Types::String))) {
					o << " : '" << node->lexVal.s << "'";
				} else {
					o << " : " << node->lexVal.i;
				}
				break;
			case Node::Type::ProcedureDef:
			case Node::Type::VarDecl:
			case Node::Type::Id:
			case Node::Type::Member:
				o << " : \'" << node->lexVal.s << "\'";
				break;
		}

		if(node->type && node->type->name != "void") {
			o << " (" << node->type->name << ")";
		}

		return o;
	}
}