#include "Analysis/AvailableExpressions.h"

#include "Analysis/DataFlow.h"

namespace Analysis {
	static IR::EntrySet emptyEntrySet; //!< Empty entry set, used when an entry lookup fails

	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 * \param flowGraph Flow graph of procedure
	 */
	AvailableExpressions::AvailableExpressions(const IR::Procedure &procedure, FlowGraph &flowGraph)
		: mProcedure(const_cast<IR::Procedure&>(procedure))
	{
		IR::EntrySet all;
		for(IR::Entry *entry : mProcedure.entries()) {
			if(isExpression(entry)) {
				all.insert(entry);
			}
		}

		EntryToEntrySetMap gen;
		EntryToEntrySetMap kill;

		for(IR::Entry *entry : mProcedure.entries()) {
			if(all.find(entry) == all.end()) {
				continue;
			}

			gen[entry].insert(entry);

			if(entry->assign()) {
				for(IR::Entry *exp : all) {
					if(exp->uses(entry->assign()) || exp->assign() == entry->assign()) {
						kill[entry].insert(exp);
					}
				}
			}
		}

		DataFlow<IR::Entry*> dataFlow;
		mExpressions = dataFlow.analyze(flowGraph, gen, kill, all, DataFlow<IR::Entry*>::Meet::Intersect, DataFlow<IR::Entry*>::Direction::Forward);
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
			case IR::Entry::Type::Add:
			case IR::Entry::Type::Subtract:
			case IR::Entry::Type::Mult:
			case IR::Entry::Type::Equal: case IR::Entry::Type::Nequal:
			case IR::Entry::Type::LessThan: case IR::Entry::Type::LessThanE:
			case IR::Entry::Type::GreaterThan: case IR::Entry::Type::GreaterThanE:
			case IR::Entry::Type::And: case IR::Entry::Type::Or:
			case IR::Entry::Type::LoadStack:
			case IR::Entry::Type::LoadMem:
			case IR::Entry::Type::StoreMem:
				return true;

			default:
				return false;
		}
	}

	/*!
	 * \brief Print out the available expression information
	 */
	void AvailableExpressions::print(std::ostream &o) const
	{
		int line = 1;
		std::map<IR::Entry*, int> lineMap;

		// Assign a line number to each entry
		for(IR::Entry *entry : mProcedure.entries()) {
			lineMap[entry] = line++;
		}

		// Iterate through the procedure, printing out each entry, along with all definitions which reach it
		for(IR::Entry *entry : mProcedure.entries()) {
			o << lineMap[entry] << ": " << *entry << " -> ";
			IR::EntrySet exps = expressions(entry);
			for(IR::Entry *e : exps) {
				o << lineMap[e] << " ";
			}
			o << std::endl;
		}
	}
}