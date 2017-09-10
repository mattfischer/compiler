#include "Transform/CommonSubexpressionElimination.h"

#include "Analysis/AvailableExpressions.h"

#include "Util/UniqueQueue.h"

namespace Transform {
	/*!
	 * \brief Find a match for an entry in a set of available expressions
	 * \param entry Entry to match
	 * \param exps Expressions
	 * \return Symbol containing entry's value
	 */
	IR::Symbol *findMatch(IR::Entry *entry, const std::set<IR::Entry*> &exps) {
		for(IR::Entry *exp : exps) {
			// Reject the expression if it is of a different type than the entry, with the
			// special exception that a memory store expression can be used to satisfy a
			// memory load entry
			if(entry->type != exp->type &&
				!(entry->type == IR::Entry::Type::LoadMem && exp->type == IR::Entry::Type::StoreMem)) {
				continue;
			}

			// Match the expression based on its type
			switch(entry->type) {
				case IR::Entry::Type::LoadStack:
				case IR::Entry::Type::LoadMem:
				case IR::Entry::Type::Add:
				case IR::Entry::Type::Subtract:
				case IR::Entry::Type::Mult:
				case IR::Entry::Type::Equal:
				case IR::Entry::Type::Nequal:
				case IR::Entry::Type::LessThan:
				case IR::Entry::Type::LessThanE:
				case IR::Entry::Type::GreaterThan:
				case IR::Entry::Type::GreaterThanE:
				case IR::Entry::Type::And:
				case IR::Entry::Type::Or:
					{
						IR::EntryThreeAddr *threeAddrEn = (IR::EntryThreeAddr*)entry;
						IR::EntryThreeAddr *threeAddrEx = (IR::EntryThreeAddr*)exp;
						if(threeAddrEn->rhs1 == threeAddrEx->rhs1 && threeAddrEn->rhs2 == threeAddrEx->rhs2 && threeAddrEn->imm == threeAddrEx->imm) {
							return threeAddrEx->lhs;
						}
						break;
					}
			}
		}

		return 0;
	}

	bool CommonSubexpressionElimination::transform(IR::Procedure &procedure, Analysis::Analysis &analysis)
	{
		Analysis::AvailableExpressions availableExpressions(procedure, analysis.flowGraph());
		Analysis::UseDefs &useDefs = analysis.useDefs();

		Util::UniqueQueue<IR::Entry*> queue;

		// Start by iterating through the entire procedure
		for(IR::Entry *entry : procedure.entries()) {
			queue.push(entry);
		}

		// Process the queue until it is empty
		bool changed = false;
		while(!queue.empty()) {
			IR::Entry *entry = queue.front();
			queue.pop();

			if(!Analysis::AvailableExpressions::isExpression(entry) || !entry->assign()) {
				continue;
			}

			// Search through all available expressions, looking for a match
			const std::set<IR::Entry*> &exps = availableExpressions.expressions(entry);
			IR::Symbol *expTarget = findMatch(entry, exps);
			if(expTarget) {
				// Replace the expression with a direct assignment to the available expression's target
				IR::Entry *newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Move, entry->assign(), expTarget);

				// Add all uses of the entry into the queue, it may now be possible
				// to do further subexpression elimination on them
				for(IR::Entry *entry : useDefs.uses(entry)) {
					queue.push(entry);
				}

				// Update the useDef chains to reflect the new entry
				analysis.replace(entry, newEntry);

				// Substitute the new entry into the procedure
				procedure.entries().insert(entry, newEntry);
				procedure.entries().erase(entry);
				delete entry;
				changed = true;
			}
		}

		return changed;
	}

	/*!
	 * \brief Singleton
	 * \return Instance
	 */
	CommonSubexpressionElimination *CommonSubexpressionElimination::instance()
	{
		static CommonSubexpressionElimination inst;
		return &inst;
	}
}