#include "Transform/ConstantProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"

#include "Util/UniqueQueue.h"

namespace Transform {
	/*!
	 * \brief Get the value of a symbol at an entry, if it can be determined to be a constant
	 * \param entry Entry to examine
	 * \param symbol Symbol to evaluate
	 * \param useDefs Use-def chains for procedure
	 * \param isConstant [out] True if symbol has a constant value, false otherwise
	 * \return Value of symbol
	 */
	int ConstantProp::getValue(IR::Entry *entry, IR::Symbol *symbol, Analysis::UseDefs &useDefs, bool &isConstant)
	{
		isConstant = false;
		int ret = 0;

		// Check each definition that reaches this entry
		const IR::EntrySet &set = useDefs.defines(entry, symbol);
		for(IR::EntrySet::const_iterator it = set.begin(); it != set.end(); it++) {
			IR::Entry *def = *it;
			if(def->type != IR::Entry::TypeLoadImm) {
				// This definition is not constant, therefore the symbol can't be
				ret = 0;
				isConstant = false;
				break;
			}

			IR::EntryOneAddrImm *imm = (IR::EntryOneAddrImm*)def;
			if(isConstant && ret != imm->imm) {
				// If the definition is a constant, and is either the first constant
				// encountered, or is equal to the value previously encountered, then
				// it can still be considered a constant
				ret = 0;
				isConstant = false;
				break;
			} else {
				// Otherwise, the entry is not constant
				ret = imm->imm;
				isConstant = true;
			}
		}

		return ret;
	}

	bool ConstantProp::transform(IR::Procedure *procedure)
	{
		bool changed = false;
		Analysis::UseDefs useDefs(procedure);

		Util::UniqueQueue<IR::Entry*> queue;

		// Start by iterating through the entire procedure
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			queue.push(entry);
		}

		// Process the queue until it is empty
		while(!queue.empty()) {
			IR::Entry *entry = queue.front();
			queue.pop();

			// Examine the current entry
			switch(entry->type) {
				case IR::Entry::TypeAdd:
				case IR::Entry::TypeMult:
				case IR::Entry::TypeMove:
				case IR::Entry::TypeEqual:
				case IR::Entry::TypeNequal:
					{
						int rhs1;
						int rhs2;
						bool rhs1Const;
						bool rhs2Const;

						// Examine the right hand side arguments and determine if they are constant
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						rhs1 = getValue(threeAddr, threeAddr->rhs1, useDefs, rhs1Const);
						if(threeAddr->rhs2) {
							rhs2 = getValue(threeAddr, threeAddr->rhs2, useDefs, rhs2Const);
						} else {
							rhs2 = 0;
							rhs2Const = true;
						}

						// If both RHS symbols are constant, the entry can be evaluated
						IR::Entry *newEntry = 0;
						if(rhs1Const && rhs2Const) {
							// Calculate the value of the entry
							int value;
							switch(entry->type) {
								case IR::Entry::TypeAdd:
									value = rhs1 + rhs2;
									break;

								case IR::Entry::TypeMult:
									value = rhs1 * rhs2;
									break;

								case IR::Entry::TypeMove:
									value = rhs1;
									break;

								case IR::Entry::TypeEqual:
									value = rhs1 == rhs2;
									break;

								case IR::Entry::TypeNequal:
									value = rhs1 != rhs2;
									break;
							}

							// Create a new immediate load entry with the calculated value
							newEntry = new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, threeAddr->lhs, value);
						} else if(rhs1Const || rhs2Const) {
							// If one argument is constant and the other is not, an entry can
							// at least be turned into an Immediate entry
							int constant;
							IR::Symbol *symbol;
							if(rhs1Const) {
								constant = rhs1;
								symbol = threeAddr->rhs2;
							} else {
								constant = rhs2;
								symbol = threeAddr->rhs1;
							}

							switch(threeAddr->type) {
								case IR::Entry::TypeAdd:
									newEntry = new IR::EntryTwoAddrImm(IR::Entry::TypeAddImm, threeAddr->lhs, symbol, constant);
									break;
								case IR::Entry::TypeMult:
									newEntry = new IR::EntryTwoAddrImm(IR::Entry::TypeMultImm, threeAddr->lhs, symbol, constant);
									break;
							}
						}

						// If a new entry was created, subsitute it into the procedure
						if(newEntry) {
							// Add all uses of the entry into the queue, it may now be possible
							// to do further constant propagation on them
							const IR::EntrySet &entries = useDefs.uses(threeAddr);
							for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
								queue.push(*it);
							}

							// Update the useDef chains to reflect the new entry
							useDefs.replace(threeAddr, newEntry);

							// Substitute the new entry into the procedure
							procedure->entries().insert(threeAddr, newEntry);
							procedure->entries().erase(threeAddr);
							delete threeAddr;
							changed = true;
						}
						break;
					}
				case IR::Entry::TypeAddImm:
				case IR::Entry::TypeMultImm:
					{
						IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;
						int rhs;
						bool rhsConst;

						// Determine if the symbol on the right hand side is constant
						rhs = getValue(twoAddrImm, twoAddrImm->rhs, useDefs, rhsConst);
						if(rhsConst) {
							int value;
							switch(twoAddrImm->type) {
								case IR::Entry::TypeAddImm:
									value = rhs + twoAddrImm->imm;
									break;

								case IR::Entry::TypeMultImm:
									value = rhs * twoAddrImm->imm;
									break;
							}

							// Construct a Load Immediate entry to replace the current entry
							IR::Entry *newEntry = new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, twoAddrImm->lhs, value);
							const IR::EntrySet &entries = useDefs.uses(twoAddrImm);
							for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
								queue.push(*it);
							}

							// Replace the entry in the useDef chains and the procedure itself
							useDefs.replace(twoAddrImm, newEntry);
							procedure->entries().insert(twoAddrImm, newEntry);
							procedure->entries().erase(twoAddrImm);
							delete twoAddrImm;
							changed = true;
						}
						break;
					}
				case IR::Entry::TypeCJump:
					{
						// Check if the predicate of the conditional jump is constant
						IR::EntryCJump *cJump = (IR::EntryCJump*)entry;
						bool isConstant;
						int value = getValue(cJump, cJump->pred, useDefs, isConstant);
						if(!isConstant) {
							continue;
						}

						// If the predicate is constant, the conditional jump can be turned into
						// an uconditional jump.  Determine which jump target should be used, based
						// on the value of the predicate, and construct a new jump entry.
						IR::EntryJump *jump;
						if(value) {
							jump = new IR::EntryJump(cJump->trueTarget);
						} else {
							jump = new IR::EntryJump(cJump->falseTarget);
						}

						// Update the useDef chains and the procedure itself
						useDefs.replace(cJump, jump);
						procedure->entries().insert(cJump, jump);
						procedure->entries().erase(cJump);
						delete cJump;
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
	ConstantProp *ConstantProp::instance()
	{
		static ConstantProp inst;
		return &inst;
	}
}