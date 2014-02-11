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
	IR::Symbol *findMatch(IR::Entry *entry, const IR::EntrySet &exps) {
		for(IR::EntrySet::const_iterator expIt = exps.begin(); expIt != exps.end(); expIt++) {
			IR::Entry *exp = *expIt;

			// Reject the expression if it is of a different type than the entry, with the
			// special exception that a memory store expression can be used to satisfy a
			// memory load entry
			if(entry->type != exp->type &&
			   !(entry->type == IR::Entry::TypeLoadMem && exp->type == IR::Entry::TypeStoreMem) &&
			   !(entry->type == IR::Entry::TypeLoadMemInd && exp->type == IR::Entry::TypeStoreMemInd)) {
				continue;
			}

			// Match the expression based on its type
			switch(entry->type) {
				case IR::Entry::TypeLoadImm:
				case IR::Entry::TypeAddImm:
				case IR::Entry::TypeMultImm:
				case IR::Entry::TypeLoadStack:
				case IR::Entry::TypeLoadMem:
				case IR::Entry::TypeAdd:
				case IR::Entry::TypeSubtract:
				case IR::Entry::TypeMult:
				case IR::Entry::TypeEqual:
				case IR::Entry::TypeNequal:
				case IR::Entry::TypeLessThan:
				case IR::Entry::TypeLessThanE:
				case IR::Entry::TypeGreaterThan:
				case IR::Entry::TypeGreaterThanE:
				case IR::Entry::TypeAnd:
				case IR::Entry::TypeOr:
				case IR::Entry::TypeLoadMemInd:
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

	bool CommonSubexpressionElimination::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		Analysis::AvailableExpressions availableExpressions(procedure, analysis.flowGraph());
		Analysis::UseDefs *useDefs = analysis.useDefs();

		Util::UniqueQueue<IR::Entry*> queue;

		// Start by iterating through the entire procedure
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
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
			const IR::EntrySet &exps = availableExpressions.expressions(entry);
			IR::Symbol *expTarget = findMatch(entry, exps);
			if(expTarget) {
				// Replace the expression with a direct assignment to the available expression's target
				IR::Entry *newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMove, entry->assign(), expTarget);

				// Add all uses of the entry into the queue, it may now be possible
				// to do further subexpression elimination on them
				const IR::EntrySet &entries = useDefs->uses(entry);
				for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
					queue.push(*it);
				}

				// Update the useDef chains to reflect the new entry
				analysis.replace(entry, newEntry);

				// Substitute the new entry into the procedure
				procedure->entries().insert(entry, newEntry);
				procedure->entries().erase(entry);
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