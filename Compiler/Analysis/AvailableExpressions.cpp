#include "Analysis/AvailableExpressions.h"

#include "Analysis/DataFlow.h"

namespace Analysis {
	static IR::EntrySet emptyEntrySet; //!< Empty entry set, used when an entry lookup fails

	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 * \param flowGraph Flow graph of procedure
	 */
	AvailableExpressions::AvailableExpressions(IR::Procedure *procedure, FlowGraph *flowGraph)
	{
		mProcedure = procedure;

		IR::EntrySet all;
		for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
			IR::Entry *entry = *entryIt;

			if(isExpression(entry)) {
				all.insert(entry);
			}
		}

		EntryToEntrySetMap gen;
		EntryToEntrySetMap kill;

		for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
			IR::Entry *entry = *entryIt;

			if(all.find(entry) == all.end()) {
				continue;
			}

			gen[entry].insert(entry);

			if(entry->assign()) {
				for(IR::EntrySet::iterator expIt = all.begin(); expIt != all.end(); expIt++) {
					IR::Entry *exp = *expIt;
					if(exp->uses(entry->assign()) || exp->assign() == entry->assign()) {
						kill[entry].insert(exp);
					}
				}
			}
		}

		DataFlow<IR::Entry*> dataFlow;
		mExpressions = dataFlow.analyze(flowGraph, gen, kill, all, DataFlow<IR::Entry*>::MeetTypeIntersect, DataFlow<IR::Entry*>::DirectionForward);
	}

	/*!
	 * \brief List the expressions available at a given point
	 * \param entry Entry in procedure
	 * \return List of available expressions
	 */
	const IR::EntrySet &AvailableExpressions::expressions(IR::Entry *entry) const
	{
		EntryToEntrySetMap::const_iterator it = mExpressions.find(entry);
		if(it != mExpressions.end()) {
			return it->second;
		} else {
			return emptyEntrySet;
		}
	}

	bool AvailableExpressions::isExpression(IR::Entry *entry)
	{
		switch(entry->type) {
			case IR::Entry::TypeLoadImm:
			case IR::Entry::TypeAdd: case IR::Entry::TypeAddImm:
			case IR::Entry::TypeSubtract:
			case IR::Entry::TypeMult: case IR::Entry::TypeMultImm:
			case IR::Entry::TypeEqual: case IR::Entry::TypeNequal:
			case IR::Entry::TypeLessThan: case IR::Entry::TypeLessThanE:
			case IR::Entry::TypeGreaterThan: case IR::Entry::TypeGreaterThanE:
			case IR::Entry::TypeAnd: case IR::Entry::TypeOr:
			case IR::Entry::TypeLoadStack:
			case IR::Entry::TypeLoadMemInd: case IR::Entry::TypeLoadMem:
				return true;

			default:
				return false;
		}
	}

	/*!
	 * \brief Print out the available expression information
	 */
	void AvailableExpressions::print() const
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
			IR::EntrySet exps = expressions(entry);
			for(IR::EntrySet::iterator it2 = exps.begin(); it2 != exps.end(); it2++) {
				IR::Entry *e = *it2;
				std::cout << lineMap[e] << " ";
			}
			std::cout << std::endl;
		}
	}
}