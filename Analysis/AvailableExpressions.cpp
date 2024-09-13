#include "Analysis/AvailableExpressions.h"

#include "Analysis/DataFlow.h"

namespace Analysis {
	static std::set<const IR::Entry*> emptyEntrySet; //!< Empty entry set, used when an entry lookup fails

	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 * \param flowGraph Flow graph of procedure
	 */
	AvailableExpressions::AvailableExpressions(const IR::Procedure &procedure, const FlowGraph &flowGraph)
		: mProcedure(procedure)
	{
		std::set<const IR::Entry*> all;
		for(const IR::Entry *entry : mProcedure.entries()) {
			if(isExpression(entry)) {
				all.insert(entry);
			}
		}

		std::map<const IR::Entry*, std::set<const IR::Entry*>> gen;
		std::map<const IR::Entry*, std::set<const IR::Entry*>> kill;

		for(const IR::Entry *entry : mProcedure.entries()) {
			if(all.find(entry) == all.end()) {
				continue;
			}

			gen[entry].insert(entry);

			if(entry->assign()) {
				for(const IR::Entry *exp : all) {
					if(exp->uses(entry->assign()) || exp->assign() == entry->assign()) {
						kill[entry].insert(exp);
					}
				}
			}
		}

		DataFlow<const IR::Entry*> dataFlow;
		mExpressions = dataFlow.analyze(flowGraph, gen, kill, all, DataFlow<const IR::Entry*>::Meet::Intersect, DataFlow<const IR::Entry*>::Direction::Forward);
	}

	/*!
	 * \brief List the expressions available at a given point
	 * \param entry Entry in procedure
	 * \return List of available expressions
	 */
	const std::set<const IR::Entry*> &AvailableExpressions::expressions(const IR::Entry *entry) const
	{
		const auto it = mExpressions.find(entry);
		if(it != mExpressions.end()) {
			return it->second;
		} else {
			return emptyEntrySet;
		}
	}

	bool AvailableExpressions::isExpression(const IR::Entry *entry)
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
		std::map<const IR::Entry*, int> lineMap;

		// Assign a line number to each entry
		for(const IR::Entry *entry : mProcedure.entries()) {
			lineMap[entry] = line++;
		}

		// Iterate through the procedure, printing out each entry, along with all definitions which reach it
		for(const IR::Entry *entry : mProcedure.entries()) {
			o << lineMap[entry] << ": " << *entry << " -> ";
			std::set<const IR::Entry*> exps = expressions(entry);
			for(const IR::Entry *e : exps) {
				o << lineMap[e] << " ";
			}
			o << std::endl;
		}
	}
}