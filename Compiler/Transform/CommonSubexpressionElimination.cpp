#include "Transform/CommonSubexpressionElimination.h"

#include "Analysis/AvailableExpressions.h"

#include "Util/UniqueQueue.h"

namespace Transform {
	/*!
	 * \brief Determine whether two expressions compute the same value
	 * \param entry1 First entry
	 * \param entry2 Second entry
	 * \return True if expressions match
	 */
	bool match(IR::Entry *entry1, IR::Entry *entry2) {
		if(entry1->type != entry2->type) {
			return false;
		}

		switch(entry1->type) {
			case IR::Entry::TypeLoadImm:
			case IR::Entry::TypeAddImm:
			case IR::Entry::TypeMultImm:
			case IR::Entry::TypeLoadStack:
			case IR::Entry::TypeLoadMem:
				{
					IR::EntryTwoAddrImm *twoAddrImm1 = (IR::EntryTwoAddrImm*)entry1;
					IR::EntryTwoAddrImm *twoAddrImm2 = (IR::EntryTwoAddrImm*)entry2;
					return (twoAddrImm1->rhs == twoAddrImm2->rhs && twoAddrImm1->imm == twoAddrImm2->imm);
				}

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
					IR::EntryThreeAddr *threeAddr1 = (IR::EntryThreeAddr*)entry1;
					IR::EntryThreeAddr *threeAddr2 = (IR::EntryThreeAddr*)entry2;
					return (threeAddr1->rhs1 == threeAddr2->rhs1 && threeAddr1->rhs2 == threeAddr2->rhs2);
				}

			default:
				return false;
		}
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

			if(!Analysis::AvailableExpressions::isExpression(entry)) {
				continue;
			}

			// Search through all available expressions, looking for a match
			const IR::EntrySet &exps = availableExpressions.expressions(entry);
			for(IR::EntrySet::const_iterator expIt = exps.begin(); expIt != exps.end(); expIt++) {
				IR::Entry *exp = *expIt;

				if(match(exp, entry)) {
					// Replace the expression with a direct assignment to the available expression's target
					IR::Entry *newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMove, entry->assign(), exp->assign());

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
					break;
				}
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