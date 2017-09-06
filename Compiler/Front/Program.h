#ifndef FRONT_PROGRAM_H
#define FRONT_PROGRAM_H

#include "Front/Scope.h"
#include "Front/Procedure.h"
#include "Front/Types.h"

#include <iostream>
#include <vector>
#include <memory>

namespace Front {
	/*!
	 * \brief A parsed and type-checked program
	 */
	struct Program {
		std::unique_ptr<Scope> scope; //!< Global variables
		std::unique_ptr<Types> types; //!< Type list
		std::vector<std::unique_ptr<Procedure>> procedures; //!< List of procedures

		void print(std::ostream &o);
	};
}

#endif