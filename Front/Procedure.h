#ifndef FRONT_PROCEDURE_H
#define FRONT_PROCEDURE_H

#include "Front/Type.h"
#include "Front/Scope.h"
#include "Front/Node.h"

#include <string>
#include <iostream>

namespace Front {
	/*!
	 * \brief A parsed and type-checked procedure
	 */
	struct Procedure {
		std::string name; //!< Procedure name
		Scope *scope; //!< Local variables
		std::vector<Symbol*> arguments; //!< List of arguments
		std::shared_ptr<TypeProcedure> type; //!< Procedure type
		Node *body; //!< Body of procedure
		Symbol *object; //!< Object symbol if procedure is a member function, or 0

		void print(std::ostream &o);
	};
}
#endif