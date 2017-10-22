#ifndef ANALYSIS_USE_DEFS_H
#define ANALYSIS_USE_DEFS_H

#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/ReachingDefs.h"

#include <map>
#include <set>
#include <vector>
#include <iostream>

namespace Analysis {
	/*!
	 * \brief Determine a set of def-use and use-def chains
	 *
	 * A use-def chain shows all definitions which can affect a given use of a variable.
	 * Similarly, a def-use chain shows all uses which may make use of a given definition.
	 */
	class UseDefs
	{
	public:
		UseDefs(const IR::Procedure &procedure, ReachingDefs &reachingDefs);

		const std::set<const IR::Entry*> &uses(const IR::Entry *def) const;
		const std::set<const IR::Entry*> &defines(const IR::Entry *use, IR::Symbol *symbol) const;

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol);
		void remove(IR::Entry *entry);

		void print(std::ostream &o) const;

	private:
		std::map<const IR::Entry*, std::set<const IR::Entry*>> mUses; //!< List of all uses
		std::map<const IR::Entry*, std::map<IR::Symbol*, std::set<const IR::Entry*>>> mDefines; //!< List of all defines
		const IR::Procedure &mProcedure; //!< Procedure being analyzed
		ReachingDefs &mReachingDefs; //!< Reaching def information for the procedure
	};
}
#endif
