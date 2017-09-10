#include "ReachingDefs.h"

#include "Analysis/FlowGraph.h"
#include "Analysis/DataFlow.h"

#include "IR/Entry.h"
#include "IR/Procedure.h"

#include "Util/UniqueQueue.h"
#include "Util/Timer.h"
#include "Util/Log.h"

namespace Analysis {
	static std::set<IR::Entry*> emptyEntrySet; //!< Empty entry set, used when an entry lookup fails

	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 */
	ReachingDefs::ReachingDefs(const IR::Procedure &procedure, FlowGraph &flowGraph)
		: mFlowGraph(flowGraph),
		mProcedure(procedure)
	{
		Util::Timer timer;
		timer.start();

		// Find all definitions in the procedure
		std::set<IR::Entry*> allDefs;
		for(IR::Entry *entry : const_cast<IR::Procedure&>(mProcedure).entries()) {
			if(entry->assign()) {
				allDefs.insert(entry);
			}
		}

		// Construct gen and kill sets for data flow analysis
		std::map<IR::Entry*, std::set<IR::Entry*>> gen;
		std::map<IR::Entry*, std::set<IR::Entry*>> kill;
		for(IR::Entry *entry : const_cast<IR::Procedure&>(mProcedure).entries()) {
			if(!entry->assign()) {
				continue;
			}

			if(entry->assign()) {
				// Assignment to a variable adds that variable to the set of reaching definitions
				gen[entry].insert(entry);
			}

			// An assignment also kills any other assignment to the same variable
			for(IR::Entry *def : allDefs) {
				if(def->assign() == entry->assign() && def != entry) {
					kill[entry].insert(def);
				}
			}
		}

		// Perform a forward data flow analysis using the gen/kill sets constructed above
		DataFlow<IR::Entry*> dataFlow;
		mDefs = dataFlow.analyze(mFlowGraph, gen, kill, allDefs, DataFlow<IR::Entry*>::Meet::Union, DataFlow<IR::Entry*>::Direction::Forward);
		Util::log("opt.time") << "  ReachingDefs(" << procedure.name() << "): " << timer.stop() << "ms" << std::endl;
	}

	/*!
	 * \brief Return the set of definitions which reach a given entry
	 * \param entry Entry to examine
	 * \return Reaching definitions for that entry
	 */
	const std::set<IR::Entry*> &ReachingDefs::defs(IR::Entry *entry) const
	{
		std::map<IR::Entry*, std::set<IR::Entry*>>::const_iterator it = mDefs.find(entry);
		if(it != mDefs.end()) {
			return it->second;
		} else {
			return emptyEntrySet;
		}
	}

	/*!
	 * \brief Convenience function to return all reaching definitions which assign the given symbol
	 * \param entry Entry to examine
	 * \param symbol Symbol to search for
	 * \return All reaching definitions which assign to the given symbol
	 */
	const std::set<IR::Entry*> ReachingDefs::defsForSymbol(IR::Entry *entry, IR::Symbol *symbol) const
	{
		std::set<IR::Entry*> result;
		const std::set<IR::Entry*> &all = defs(entry);
		for(IR::Entry *def : all) {
			if(def->assign() == symbol) {
				result.insert(def);
			}
		}

		return result;
	}

	/*!
	 * \brief Replace an entry with another
	 * \param oldEntry Existing entry
	 * \param newEntry Entry to replace it with
	 */
	void ReachingDefs::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		// Assign the reaching definitions from the old entry to the new one, and remove
		// the old one from the set
		mDefs[newEntry] = mDefs[oldEntry];
		mDefs.erase(oldEntry);

		// Search through all reaching definitions, and replace the old entry with the new one
		for(auto &def : mDefs) {
			std::set<IR::Entry*> &set = def.second;
			std::set<IR::Entry*>::iterator setIt = set.find(oldEntry);
			if(setIt != set.end()) {
				set.erase(setIt);
				set.insert(newEntry);
			}
		}
	}

	/*!
	 * \brief Remove an entry
	 * \param entry Entry to remove
	 */
	void ReachingDefs::remove(IR::Entry *entry)
	{
		// Remove all references to the given entry
		for(auto &def : mDefs) {
			std::set<IR::Entry*> &set = def.second;
			std::set<IR::Entry*>::iterator setIt = set.find(entry);
			if(setIt != set.end()) {
				set.erase(setIt);
			}
		}
		mDefs.erase(entry);
	}

	/*!
	 * \brief Print out the reaching definition information
	 */
	void ReachingDefs::print(std::ostream &o) const
	{
		int line = 1;
		std::map<IR::Entry*, int> lineMap;

		// Assign a line number to each entry
		for(IR::Entry *entry : const_cast<IR::Procedure&>(mProcedure).entries()) {
			lineMap[entry] = line++;
		}

		// Iterate through the procedure, printing out each entry, along with all definitions which reach it
		for(IR::Entry *entry : const_cast<IR::Procedure&>(mProcedure).entries()) {
			o << lineMap[entry] << ": " << *entry << " -> ";
			std::set<IR::Entry*> d = defs(entry);
			for(IR::Entry *e : d) {
				o << lineMap[e] << " ";
			}
			o << std::endl;
		}
	}
}