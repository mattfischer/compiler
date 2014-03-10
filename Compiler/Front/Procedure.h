#ifndef FRONT_PROCEDURE_H
#define FRONT_PROCEDURE_H

#include "Front/Type.h"
#include "Front/Scope.h"
#include "Front/Node.h"

#include <string>

namespace Front {
	/*!
	 * \brief A parsed and type-checked procedure
	 */
	struct Procedure {
		std::string name; //!< Procedure name
		Scope *locals; //!< Local variables
		std::vector<Symbol*> arguments; //!< List of arguments
		TypeProcedure *type; //!< Procedure type
		Node *body; //!< Body of procedure
		Type *classType; //!< Containing class, or 0 or non-class procedure

		void print();
	};
}
#endif