#ifndef FRONT_PROGRAM_H
#define FRONT_PROGRAM_H

#include "Front/Scope.h"
#include "Front/Procedure.h"

#include <vector>
namespace Front {
	/*!
	 * \brief A parsed and type-checked program
	 */
	struct Program {
		Scope *globals; //!< Global variables
		std::vector<Procedure*> procedures; //!< List of procedures

		void print();
	};
}

#endif