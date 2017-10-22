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
		ReachingDefs(const IR::Procedure &procedure, FlowGraph &flowGraph);

		const std::set<const IR::Entry*> &defs(const IR::Entry* entry) const;
		const std::set<const IR::Entry*> defsForSymbol(const IR::Entry* entry, IR::Symbol *symbol) const;
		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void remove(IR::Entry *entry);
		void print(std::ostream &o) const;

	private:
		FlowGraph &mFlowGraph; //<! Flow graph being analyzed
		std::map<const IR::Entry*, std::set<const IR::Entry*>> mDefs; //!< List of definitions
		const IR::Procedure &mProcedure; //!< Procedure being analyzed
	};
}
#endif