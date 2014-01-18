#ifndef ANALYSIS_USE_DEFS_H
#define ANALYSIS_USE_DEFS_H

#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/ReachingDefs.h"

#include <map>
#include <set>
#include <vector>

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
		UseDefs(IR::Procedure *procedure);

		const IR::EntrySet &uses(IR::Entry *def) const;
		const IR::EntrySet &defines(IR::Entry *use, IR::Symbol *symbol) const;

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol);
		void remove(IR::Entry *entry);

		void print() const;

	private:
		typedef std::map<IR::Symbol*, IR::EntrySet> SymbolToEntrySetMap;

		std::map<IR::Entry*, IR::EntrySet> mUses; //!< List of all uses
		std::map<IR::Entry*, SymbolToEntrySetMap> mDefines; //!< List of all defines
		IR::Procedure *mProcedure; //!< Procedure being analyzed
		ReachingDefs mReachingDefs; //!< Reaching def information for the procedure
	};
}
#endif
