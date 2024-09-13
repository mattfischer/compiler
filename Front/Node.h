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
		enum class Type {
			List, //!< Generic list of nodes
			ProcedureDef, //!< Procedure definition
			VarDecl, //!< Variable declaration
			StructDef, //!< Structure definition
			ClassDef, //!< Class definition
			ClassMember, //!< Class member
			Return, //!< Return statement
			Assign, //!< Assignment operator
			If, //!< If statement
			While, //!< While statement
			For, //!< For statement
			Compare, //!< Comparison operators
			Arith, //!< Arithmetic operators
			Constant, //!< Numeric constant
			Id, //!< Identifier
			Call, //!< Procedure call
			Array, //!< Array type
			New, //!< Allocate new object
			Break, //!< Break statement
			Continue, //!< Continue statement
			Member, //!< Structure member
			Coerce, //!< Convert argument to different type
			Qualifier //!< Member qualifier
		};

		/*!
		 * \brief Node subtype, for node types which require it
		 */
		enum Subtype {
			None, //!< No subtype

			Add, //!< Addition operator
			Subtract, //!< Subtraction operator
			Multiply, //!< Multiplication operator
			Divide, //!< Division operator
			Modulo, //!< Modulo operator
			Increment, //!< Increment operator
			Decrement, //!< Decrement operator

			Equal, //!< Equality comparison
			Nequal, //!< Inequality comparison
			LessThan, //!< Less-than comparison
			LessThanEqual, //!< Less-than-equal comparison
			GreaterThan, //!< Greater-than comparison
			GreaterThanEqual, //!< Greater-than-equal comparison
			Or, //!< Or comparison
			And, //!< And comparison

			Virtual, //!< Virtual function
			Native, //!< Native function
			Static, //!< Static function
		};

		Node::Type nodeType; //!< Node type
		Node::Subtype nodeSubtype; //!< Node subtype
		int line; //!< Line in source file corresponding to node
		std::vector<std::unique_ptr<Node>> children; //!< Children of node
		std::shared_ptr<Front::Type> type; //!< Assigned type of language element represented by node

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

	std::ostream &operator<<(std::ostream &o, const Node &node);
}

#endif