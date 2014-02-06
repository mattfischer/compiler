#ifndef FRONT_NODE_H
#define FRONT_NODE_H

#include "Front/Type.h"
#include "Front/Symbol.h"

#include <vector>
#include <iostream>

namespace Front {
	/*!
	 * \brief Abstract syntax tree node
	 */
	struct Node {
		/*!
		 * \brief Node type
		 */
		enum NodeType {
			NodeTypeList, //!< Generic list of nodes
			NodeTypeProcedureDef, //!< Procedure definition
			NodeTypeVarDecl, //!< Variable declaration
			NodeTypeStruct, //!< Structure definition
			NodeTypeReturn, //!< Return statement
			NodeTypePrint, //!< Print statement
			NodeTypeAssign, //!< Assignment operator
			NodeTypeIf, //!< If statement
			NodeTypeWhile, //!< While statement
			NodeTypeFor, //!< For statement
			NodeTypeCompare, //!< Comparison operators
			NodeTypeArith, //!< Arithmetic operators
			NodeTypeConstant, //!< Numeric constant
			NodeTypeId, //!< Identifier
			NodeTypeCall, //!< Procedure call
			NodeTypeArray, //!< Array type
			NodeTypeNew, //!< Allocate new object
			NodeTypeBreak, //!< Break statement
			NodeTypeContinue //!< Continue statement
		};

		/*!
		 * \brief Node subtype, for node types which require it
		 */
		enum NodeSubtype {
			NodeSubtypeNone, //!< No subtype

			NodeSubtypeAdd, //!< Addition operator
			NodeSubtypeSubtract, //!< Subtraction operator
			NodeSubtypeMultiply, //!< Multiplication operator
			NodeSubtypeIncrement, //!< Increment operator
			NodeSubtypeDecrement, //!< Decrement operator

			NodeSubtypeEqual, //!< Equality comparison
			NodeSubtypeNequal, //!< Inequality comparison
			NodeSubtypeLessThan, //!< Less-than comparison
			NodeSubtypeLessThanEqual, //!< Less-than-equal comparison
			NodeSubtypeGreaterThan, //!< Greater-than comparison
			NodeSubtypeGreaterThanEqual, //!< Greater-than-equal comparison
			NodeSubtypeOr, //!< Or comparison
			NodeSubtypeAnd, //!< And comparison
		};

		NodeType nodeType; //!< Node type
		NodeSubtype nodeSubtype; //!< Node subtype
		int line; //!< Line in source file corresponding to node
		std::vector<Node*> children; //!< Children of node
		Type *type; //!< Assigned type of language element represented by node

		/*!
		 * \brief Structure containing various lexical types that can be associated with the node
		 */
		struct LexVal {
			int i; //!< Integer value
			std::string s; //!< String value
		};
		LexVal lexVal; //!< Lexical value from tokenizer

		Symbol *symbol; //!< Symbol the node corresponds to
	};

	std::ostream &operator<<(std::ostream &o, Node *node);
}

#endif