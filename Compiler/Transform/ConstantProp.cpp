#include "Transform/ConstantProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"
#include "Analysis/Constants.h"

#include "Util/UniqueQueue.h"

namespace Transform {
	bool ConstantProp::transform(IR::Procedure *procedure)
	{
		bool changed = false;
		Analysis::UseDefs useDefs(procedure);
		Analysis::Constants constants(procedure);

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
						rhs1 = constants.getValue(threeAddr, threeAddr->rhs1, rhs1Const);
						if(threeAddr->rhs2) {
							rhs2 = constants.getValue(threeAddr, threeAddr->rhs2, rhs2Const);
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
							newEntry = new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, threeAddr->lhs, 0, value);
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
									if(constant == 0) {
										newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMove, threeAddr->lhs, symbol);
									} else {
										newEntry = new IR::EntryTwoAddrImm(IR::Entry::TypeAddImm, threeAddr->lhs, symbol, constant);
									}
									break;
								case IR::Entry::TypeMult:
									if(constant == 1) {
										newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMove, threeAddr->lhs, symbol);
									} else {
										newEntry = new IR::EntryTwoAddrImm(IR::Entry::TypeMultImm, threeAddr->lhs, symbol, constant);
									}
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
						rhs = constants.getValue(twoAddrImm, twoAddrImm->rhs, rhsConst);
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
							IR::Entry *newEntry = new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, twoAddrImm->lhs, 0, value);
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
				case IR::Entry::TypeLoadMemInd:
				case IR::Entry::TypeStoreMemInd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						bool isConstant;
						int value = constants.getValue(threeAddr, threeAddr->rhs2, isConstant);
						if(isConstant) {
							IR::Entry::Type type;
							switch(entry->type) {
								case IR::Entry::TypeLoadMemInd:
									type = IR::Entry::TypeLoadMem;
									break;
								case IR::Entry::TypeStoreMemInd:
									type = IR::Entry::TypeStoreMem;
									break;
							}
							IR::Entry *newEntry = new IR::EntryTwoAddrImm(type, threeAddr->lhs, threeAddr->rhs1, value);
							useDefs.replace(threeAddr, newEntry);
							procedure->entries().insert(threeAddr, newEntry);
							procedure->entries().erase(threeAddr);
							delete threeAddr;
							changed = true;
						}
						break;
					}
				case IR::Entry::TypeCJump:
					{
						// Check if the predicate of the conditional jump is constant
						IR::EntryCJump *cJump = (IR::EntryCJump*)entry;
						bool isConstant;
						int value = constants.getValue(cJump, cJump->pred, isConstant);
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