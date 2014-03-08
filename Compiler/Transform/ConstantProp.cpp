#include "Transform/ConstantProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Util/UniqueQueue.h"

#include <sstream>

namespace Transform {
	int log2(int value)
	{
		int ret = 0;
		while(value > 1) {
			value >>= 1;
			ret++;
		}

		return ret;
	}

	bool isPowerOfTwo(int value)
	{
		return ((1 << log2(value)) == value);
	}

	bool ConstantProp::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;
		bool invalidate = false;
		Analysis::UseDefs *useDefs = analysis.useDefs();
		Analysis::Constants *constants = analysis.constants();

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
				case IR::Entry::TypeSubtract:
				case IR::Entry::TypeMult:
				case IR::Entry::TypeDivide:
				case IR::Entry::TypeModulo:
				case IR::Entry::TypeMove:
				case IR::Entry::TypeEqual:
				case IR::Entry::TypeNequal:
				case IR::Entry::TypeLessThan:
				case IR::Entry::TypeLessThanE:
				case IR::Entry::TypeGreaterThan:
				case IR::Entry::TypeGreaterThanE:
				case IR::Entry::TypeOr:
				case IR::Entry::TypeAnd:
					{
						int rhs1;
						int rhs2;
						bool rhs1Const;
						bool rhs2Const;

						// Examine the right hand side arguments and determine if they are constant
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						rhs1 = constants->getIntValue(threeAddr, threeAddr->rhs1, rhs1Const);
						if(threeAddr->rhs2) {
							rhs2 = constants->getIntValue(threeAddr, threeAddr->rhs2, rhs2Const);
						} else {
							rhs2 = threeAddr->imm;
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

								case IR::Entry::TypeSubtract:
									value = rhs1 - rhs2;
									break;

								case IR::Entry::TypeMult:
									value = rhs1 * rhs2;
									break;

								case IR::Entry::TypeDivide:
									value = rhs1 / rhs2;
									break;

								case IR::Entry::TypeModulo:
									value = rhs1 % rhs2;
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

								case IR::Entry::TypeLessThan:
									value = rhs1 < rhs2;
									break;

								case IR::Entry::TypeLessThanE:
									value = rhs1 <= rhs2;
									break;

								case IR::Entry::TypeGreaterThan:
									value = rhs1 > rhs2;
									break;

								case IR::Entry::TypeGreaterThanE:
									value = rhs1 >= rhs2;
									break;

								case IR::Entry::TypeOr:
									value = (rhs1 || rhs2);
									break;

								case IR::Entry::TypeAnd:
									value = (rhs1 && rhs2);
									break;
							}

							// Create a new immediate load entry with the calculated value
							newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMove, threeAddr->lhs, 0, 0, value);
						} else if(threeAddr->rhs2 && (rhs1Const || rhs2Const)) {
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
										newEntry = new IR::EntryThreeAddr(IR::Entry::TypeAdd, threeAddr->lhs, symbol, 0, constant);
									}
									break;
								case IR::Entry::TypeSubtract:
									if(constant == 0) {
										newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMove, threeAddr->lhs, symbol);
									} else {
										newEntry = new IR::EntryThreeAddr(IR::Entry::TypeAdd, threeAddr->lhs, symbol, 0, -constant);
									}
									break;
								case IR::Entry::TypeMult:
									if(constant == 1) {
										newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMove, threeAddr->lhs, symbol);
									} else {
										newEntry = new IR::EntryThreeAddr(IR::Entry::TypeMult, threeAddr->lhs, symbol, 0, constant);
									}
									break;
							}
						}

						// If a new entry was created, substitute it into the procedure
						if(newEntry) {
							// Add all uses of the entry into the queue, it may now be possible
							// to do further constant propagation on them
							const IR::EntrySet &entries = useDefs->uses(threeAddr);
							for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
								queue.push(*it);
							}

							// Update the useDef chains to reflect the new entry
							analysis.replace(threeAddr, newEntry);

							// Substitute the new entry into the procedure
							procedure->entries().insert(threeAddr, newEntry);
							procedure->entries().erase(threeAddr);
							delete threeAddr;
							changed = true;
						}
						break;
					}

				case IR::Entry::TypeStringConcat:
					{
						std::string rhs1;
						std::string rhs2;
						bool rhs1Const;
						bool rhs2Const;

						// Examine the right hand side arguments and determine if they are constant
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						rhs1 = constants->getStringValue(threeAddr, threeAddr->rhs1, rhs1Const);
						rhs2 = constants->getStringValue(threeAddr, threeAddr->rhs2, rhs2Const);

						if(rhs1Const && rhs2Const) {
							IR::Entry *newEntry = new IR::EntryString(IR::Entry::TypeLoadString, threeAddr->lhs, rhs1 + rhs2);

							// Add all uses of the entry into the queue, it may now be possible
							// to do further constant propagation on them
							const IR::EntrySet &entries = useDefs->uses(threeAddr);
							for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
								queue.push(*it);
							}

							// Update the useDef chains to reflect the new entry
							analysis.replace(threeAddr, newEntry);

							// Substitute the new entry into the procedure
							procedure->entries().insert(threeAddr, newEntry);
							procedure->entries().erase(threeAddr);
							delete threeAddr;
							changed = true;
						}
						break;
					}

				case IR::Entry::TypeStringBool:
				case IR::Entry::TypeStringInt:
					{
						int rhs;
						bool rhsConst;

						// Examine the right hand side arguments and determine if they are constant
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						rhs = constants->getIntValue(threeAddr, threeAddr->rhs1, rhsConst);

						if(rhsConst) {
							std::string str;
							if(entry->type == IR::Entry::TypeStringBool) {
								str = rhs ? "true" : "false";
							} else if(entry->type == IR::Entry::TypeStringInt) {
								std::stringstream s;
								s << rhs;
								str = s.str();
							}

							IR::Entry *newEntry = new IR::EntryString(IR::Entry::TypeLoadString, threeAddr->lhs, str);

							// Add all uses of the entry into the queue, it may now be possible
							// to do further constant propagation on them
							const IR::EntrySet &entries = useDefs->uses(threeAddr);
							for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
								queue.push(*it);
							}

							// Update the useDef chains to reflect the new entry
							analysis.replace(threeAddr, newEntry);

							// Substitute the new entry into the procedure
							procedure->entries().insert(threeAddr, newEntry);
							procedure->entries().erase(threeAddr);
							delete threeAddr;
							changed = true;
						}
						break;
					}

				case IR::Entry::TypeLoadMem:
				case IR::Entry::TypeStoreMem:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;

						if(!threeAddr->rhs2) {
							break;
						}

						bool isConstant;
						int value = constants->getIntValue(threeAddr, threeAddr->rhs2, isConstant);
						if(isConstant) {
							analysis.replaceUse(threeAddr, threeAddr->rhs2, 0);
							threeAddr->rhs2 = 0;
							threeAddr->imm = value;
							changed = true;
						} else {
							const IR::EntrySet &defs = useDefs->defines(entry, threeAddr->rhs2);
							if(defs.size() == 1) {
								IR::Entry *def = *(defs.begin());
								IR::EntryThreeAddr *defThreeAddr = (IR::EntryThreeAddr*)def;
								if(def->type == IR::Entry::TypeMult && !defThreeAddr->rhs2 && isPowerOfTwo(defThreeAddr->imm)) {
									analysis.replaceUse(threeAddr, threeAddr->rhs2, defThreeAddr->rhs1);
									threeAddr->rhs2 = defThreeAddr->rhs1;
									threeAddr->imm = log2(defThreeAddr->imm);
								}
							}
						}
						break;
					}
				case IR::Entry::TypeCJump:
					{
						// Check if the predicate of the conditional jump is constant
						IR::EntryCJump *cJump = (IR::EntryCJump*)entry;
						bool isConstant;
						int value = constants->getIntValue(cJump, cJump->pred, isConstant);
						if(!isConstant) {
							continue;
						}

						// If the predicate is constant, the conditional jump can be turned into
						// an unconditional jump.  Determine which jump target should be used, based
						// on the value of the predicate, and construct a new jump entry.
						IR::EntryJump *jump;
						if(value) {
							jump = new IR::EntryJump(cJump->trueTarget);
						} else {
							jump = new IR::EntryJump(cJump->falseTarget);
						}

						// Update the useDef chains and the procedure itself
						analysis.replace(cJump, jump);
						procedure->entries().insert(cJump, jump);
						procedure->entries().erase(cJump);
						delete cJump;
						changed = true;
						invalidate = true;

						break;
					}
			}
		}

		if(invalidate) {
			analysis.invalidate();
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