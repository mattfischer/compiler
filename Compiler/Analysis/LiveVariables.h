#ifndef ANALYSIS_LIVE_VARIABLES_H
#define ANALYSIS_LIVE_VARIABLES_H

#include "IR/Procedure.h"
#include "IR/Symbol.h"
#include "IR/Entry.h"

#include <set>
#include <map>

namespace Analysis {
	/*!
	 * \brief Determine the set of variables which are live at each entry in a procedure
	 *
	 * A variable is live if its value will be necessary at some later point in the procedure.
	 * It is dead if no further access will be made to the variable's current value.  Note that
	 * this means a variable can go dead and later become live again when another assignment is
	 * made to it.
	 */
	class LiveVariables {
	public:
		LiveVariables(IR::Procedure *procedure);

		IR::SymbolSet &variables(IR::Entry *entry);
		void print();

	private:
		std::map<IR::Entry*, IR::SymbolSet> mMap; //!< Map of live ranges
		IR::Procedure *mProcedure; //!< Procedure under analysis
	};
}

#endif