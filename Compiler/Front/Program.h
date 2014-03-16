#ifndef FRONT_PROGRAM_H
#define FRONT_PROGRAM_H

#include "Front/Scope.h"
#include "Front/Procedure.h"
#include "Front/Types.h"

#include <iostream>
#include <vector>
namespace Front {
	/*!
	 * \brief A parsed and type-checked program
	 */
	struct Program {
		Scope *scope; //!< Global variables
		Types *types; //!< Type list
		std::vector<Procedure*> procedures; //!< List of procedures

		void print(std::ostream &o);
	};
}

#endif