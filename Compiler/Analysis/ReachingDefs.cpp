#include "ReachingDefs.h"

#include "Analysis/FlowGraph.h"
#include "Analysis/DataFlow.h"

#include "IR/Entry.h"
#include "IR/Procedure.h"

#include "Util/UniqueQueue.h"

namespace Analysis {
	static IR::EntrySet emptyEntrySet; //!< Empty entry set, used when an entry lookup fails

	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 */
	ReachingDefs::ReachingDefs(IR::Procedure *procedure)
		: mFlowGraph(procedure), mProcedure(procedure)
	{
		// Find all definitions in the procedure
		IR::EntrySet allDefs;
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			if(entry->assign()) {
				allDefs.insert(entry);
			}
		}

		// Construct gen and kill sets for data flow analysis
		EntryToEntrySetMap gen;
		EntryToEntrySetMap kill;
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			if(!entry->assign()) {
				continue;
			}

			if(entry->assign()) {
				// Assignment to a variable adds that variable to the set of reaching definitions
				gen[entry].insert(entry);
			}

			// An assignment also kills any other assignment to the same variable
			for(IR::EntrySet::const_iterator itDef = allDefs.begin(); itDef != allDefs.end(); itDef++) {
				IR::Entry *def = *itDef;
				if(def->assign() == entry->assign() && def != entry) {
					kill[entry].insert(def);
				}
			}
		}

		// Perform a forward data flow analysis using the gen/kill sets constructed above
		DataFlow<IR::Entry*> dataFlow;
		mDefs = dataFlow.analyze(mFlowGraph, gen, kill, allDefs, DataFlow<IR::Entry*>::MeetTypeUnion, DataFlow<IR::Entry*>::DirectionForward);
	}

	/*!
	 * \brief Return the set of definitions which reach a given entry
	 * \param entry Entry to examine
	 * \return Reaching definitions for that entry
	 */
	const IR::EntrySet &ReachingDefs::defs(IR::Entry *entry) const
	{
		std::map<IR::Entry*, IR::EntrySet>::const_iterator it = mDefs.find(entry);
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
	const IR::EntrySet ReachingDefs::defsForSymbol(IR::Entry *entry, IR::Symbol *symbol) const
	{
		IR::EntrySet result;
		const IR::EntrySet &all = defs(entry);
		for(IR::EntrySet::const_iterator it = all.begin(); it != all.end(); it++) {
			IR::Entry *def = *it;
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
		for(EntryToEntrySetMap::iterator it = mDefs.begin(); it != mDefs.end(); it++) {
			IR::EntrySet &set = it->second;
			IR::EntrySet::iterator setIt = set.find(oldEntry);
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
		for(EntryToEntrySetMap::iterator it = mDefs.begin(); it != mDefs.end(); it++) {
			IR::EntrySet &set = it->second;
			IR::EntrySet::iterator setIt = set.find(entry);
			if(setIt != set.end()) {
				set.erase(setIt);
			}
		}
		mDefs.erase(entry);
	}

	/*!
	 * \brief Print out the reaching definition information
	 */
	void ReachingDefs::print() const
	{
		int line = 1;
		std::map<IR::Entry*, int> lineMap;

		// Assign a line number to each entry
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			lineMap[entry] = line++;
		}

		// Iterate through the procedure, printing out each entry, along with all definitions which reach it
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			std::cout << lineMap[entry] << ": " << *entry << " -> ";
			IR::EntrySet d = defs(entry);
			for(IR::EntrySet::iterator it2 = d.begin(); it2 != d.end(); it2++) {
				IR::Entry *e = *it2;
				std::cout << lineMap[e] << " ";
			}
			std::cout << std::endl;
		}
	}
}