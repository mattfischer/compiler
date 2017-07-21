#ifndef ANALYSIS_REACHING_DEFS_H
#define ANALYSIS_REACHING_DEFS_H

#include "IR/Symbol.h"
#include "IR/Entry.h"
#include "IR/Procedure.h"

#include "Analysis/FlowGraph.h"

#include <vector>
#include <set>
#include <map>
#include <iostream>

/*!
 * \brief Utilities to analyze a program in various ways
 */
namespace Analysis {
	/*!
	 * \brief Determine the set of definitions which reach each point in the procedure
	 *
	 * A definition reaches a given entry if the value assigned to the variable in that
	 * definition may still be present in the variable when control reaches the entry.
	 * An entry may have more than one reaching definition for each variable
	 */
	class ReachingDefs {
	public:
		ReachingDefs(IR::Procedure *procedure, FlowGraph &flowGraph);

		typedef std::map<IR::Symbol*, IR::EntrySet> SymbolToEntrySetMap;
		const IR::EntrySet &defs(IR::Entry* entry) const;
		const IR::EntrySet defsForSymbol(IR::Entry* entry, IR::Symbol *symbol) const;
		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void remove(IR::Entry *entry);
		void print(std::ostream &o) const;

	private:
		typedef std::map<IR::Entry*, IR::EntrySet> EntryToEntrySetMap;

		FlowGraph &mFlowGraph; //<! Flow graph being analyzed
		EntryToEntrySetMap mDefs; //!< List of definitions
		IR::Procedure *mProcedure; //!< Procedure being analyzed
	};
}
#endif