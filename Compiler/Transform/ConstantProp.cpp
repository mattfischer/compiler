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

	IR::EntryThreeAddr *getStoreArg(IR::EntryCall *call, int arg)
	{
		IR::Entry *entry = call->prev;
		while(true) {
			if(entry->type == IR::Entry::Type::StoreArg && ((IR::EntryThreeAddr*)entry)->imm == arg) {
				return (IR::EntryThreeAddr*)entry;
			} else {
				entry = entry->prev;
			}
		}
	}

	IR::EntryThreeAddr *getLoadRet(IR::EntryCall *call)
	{
		IR::Entry *entry = call->prev;
		while(true) {
			if(entry->type == IR::Entry::Type::LoadRet) {
				return (IR::EntryThreeAddr*)entry;
			} else {
				entry = entry->next;
			}
		}
	}

	void replaceCall(IR::EntryList &entries, IR::Entry *callEntry, IR::Entry *newEntry)
	{
		IR::Entry *entry = callEntry->prev;
		while(entry->type == IR::Entry::Type::StoreArg) {
			IR::Entry *toDelete = entry;
			entry = entry->prev;
			entries.erase(toDelete);
			delete toDelete;
		}

		entry = callEntry->next;
		while(entry->type == IR::Entry::Type::LoadRet) {
			IR::Entry *toDelete = entry;
			entry = entry->next;
			entries.erase(toDelete);
			delete toDelete;
		}

		entries.insert(callEntry, newEntry);
		entries.erase(callEntry);
		delete callEntry;
	}

	bool ConstantProp::transform(IR::Procedure &procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;
		bool invalidate = false;
		Analysis::UseDefs &useDefs = analysis.useDefs();
		Analysis::Constants &constants = analysis.constants();

		Util::UniqueQueue<IR::Entry*> queue;

		// Start by iterating through the entire procedure
		for(IR::Entry *entry : procedure.entries()) {
			queue.push(entry);
		}

		// Process the queue until it is empty
		while(!queue.empty()) {
			IR::Entry *entry = queue.front();
			queue.pop();

			// Examine the current entry
			switch(entry->type) {
				case IR::Entry::Type::Add:
				case IR::Entry::Type::Subtract:
				case IR::Entry::Type::Mult:
				case IR::Entry::Type::Divide:
				case IR::Entry::Type::Modulo:
				case IR::Entry::Type::Move:
				case IR::Entry::Type::Equal:
				case IR::Entry::Type::Nequal:
				case IR::Entry::Type::LessThan:
				case IR::Entry::Type::LessThanE:
				case IR::Entry::Type::GreaterThan:
				case IR::Entry::Type::GreaterThanE:
				case IR::Entry::Type::Or:
				case IR::Entry::Type::And:
					{
						int rhs1;
						int rhs2;
						bool rhs1Const;
						bool rhs2Const;

						// Examine the right hand side arguments and determine if they are constant
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						rhs1 = constants.getIntValue(threeAddr, threeAddr->rhs1, rhs1Const);
						if(threeAddr->rhs2) {
							rhs2 = constants.getIntValue(threeAddr, threeAddr->rhs2, rhs2Const);
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
								case IR::Entry::Type::Add:
									value = rhs1 + rhs2;
									break;

								case IR::Entry::Type::Subtract:
									value = rhs1 - rhs2;
									break;

								case IR::Entry::Type::Mult:
									value = rhs1 * rhs2;
									break;

								case IR::Entry::Type::Divide:
									value = rhs1 / rhs2;
									break;

								case IR::Entry::Type::Modulo:
									value = rhs1 % rhs2;
									break;

								case IR::Entry::Type::Move:
									value = rhs1;
									break;

								case IR::Entry::Type::Equal:
									value = rhs1 == rhs2;
									break;

								case IR::Entry::Type::Nequal:
									value = rhs1 != rhs2;
									break;

								case IR::Entry::Type::LessThan:
									value = rhs1 < rhs2;
									break;

								case IR::Entry::Type::LessThanE:
									value = rhs1 <= rhs2;
									break;

								case IR::Entry::Type::GreaterThan:
									value = rhs1 > rhs2;
									break;

								case IR::Entry::Type::GreaterThanE:
									value = rhs1 >= rhs2;
									break;

								case IR::Entry::Type::Or:
									value = (rhs1 || rhs2);
									break;

								case IR::Entry::Type::And:
									value = (rhs1 && rhs2);
									break;
							}

							// Create a new immediate load entry with the calculated value
							newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Move, threeAddr->lhs, 0, 0, value);
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
								case IR::Entry::Type::Add:
									if(constant == 0) {
										newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Move, threeAddr->lhs, symbol);
									} else {
										newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Add, threeAddr->lhs, symbol, 0, constant);
									}
									break;
								case IR::Entry::Type::Subtract:
									if(constant == 0) {
										newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Move, threeAddr->lhs, symbol);
									} else {
										newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Add, threeAddr->lhs, symbol, 0, -constant);
									}
									break;
								case IR::Entry::Type::Mult:
									if(constant == 1) {
										newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Move, threeAddr->lhs, symbol);
									} else {
										newEntry = new IR::EntryThreeAddr(IR::Entry::Type::Mult, threeAddr->lhs, symbol, 0, constant);
									}
									break;
							}
						}

						// If a new entry was created, substitute it into the procedure
						if(newEntry) {
							// Add all uses of the entry into the queue, it may now be possible
							// to do further constant propagation on them
							for(const IR::Entry *entry : useDefs.uses(entry)) {
								queue.push(const_cast<IR::Entry*>(entry));
							}

							// Update the useDef chains to reflect the new entry
							analysis.replace(threeAddr, newEntry);

							// Substitute the new entry into the procedure
							procedure.entries().insert(threeAddr, newEntry);
							procedure.entries().erase(threeAddr);
							delete threeAddr;
							changed = true;
						}
						break;
					}

				case IR::Entry::Type::Call:
					{
						IR::EntryCall *call = (IR::EntryCall*)entry;

						if(call->target == "__string_concat") {
							IR::EntryThreeAddr *rhs1Entry = getStoreArg(call, 0);
							IR::EntryThreeAddr *rhs2Entry = getStoreArg(call, 1);
							IR::EntryThreeAddr *retEntry = getLoadRet(call);

							std::string rhs1;
							std::string rhs2;
							bool rhs1Const;
							bool rhs2Const;

							rhs1 = constants.getStringValue(rhs1Entry, rhs1Entry->rhs1, rhs1Const);
							rhs2 = constants.getStringValue(rhs2Entry, rhs2Entry->rhs1, rhs2Const);

							if(rhs1Const && rhs2Const) {
								IR::Entry *newEntry = new IR::EntryString(IR::Entry::Type::LoadString, retEntry->lhs, rhs1 + rhs2);

								// Add all uses of the entry into the queue, it may now be possible
								// to do further constant propagation on them
								for(const IR::Entry *entry : useDefs.uses(call)) {
									queue.push(const_cast<IR::Entry*>(entry));
								}

								// Update the useDef chains to reflect the new entry
								analysis.remove(rhs1Entry);
								analysis.remove(rhs2Entry);
								analysis.replace(retEntry, newEntry);

								replaceCall(procedure.entries(), call, newEntry);
								changed = true;
							}
							break;
						} else if(call->target == "__string_int" || call->target == "__string_bool" || call->target == "__string_char") {
							IR::EntryThreeAddr *rhsEntry = getStoreArg(call, 0);
							IR::EntryThreeAddr *retEntry = getLoadRet(call);

							int rhs;
							bool rhsConst;

							rhs = constants.getIntValue(rhsEntry, rhsEntry->rhs1, rhsConst);

							if(rhsConst) {
								std::string str;
								if(call->target == "__string_bool") {
									str = rhs ? "true" : "false";
								} else if(call->target == "__string_int") {
									std::stringstream s;
									s << rhs;
									str = s.str();
								} else if(call->target == "__string_char") {
									str = (char)rhs;
								}

								IR::Entry *newEntry = new IR::EntryString(IR::Entry::Type::LoadString, retEntry->lhs, str);

								// Add all uses of the entry into the queue, it may now be possible
								// to do further constant propagation on them
								for(const IR::Entry *entry : useDefs.uses(call)) {
									queue.push(const_cast<IR::Entry*>(entry));
								}

								// Update the useDef chains to reflect the new entry
								analysis.remove(rhsEntry);
								analysis.replace(retEntry, newEntry);

								replaceCall(procedure.entries(), call, newEntry);
								changed = true;
							}
							break;
						}
					}

				case IR::Entry::Type::LoadMem:
				case IR::Entry::Type::StoreMem:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;

						if(!threeAddr->rhs2) {
							break;
						}

						bool isConstant;
						int value = constants.getIntValue(threeAddr, threeAddr->rhs2, isConstant);
						if(isConstant) {
							analysis.replaceUse(threeAddr, threeAddr->rhs2, 0);
							threeAddr->rhs2 = 0;
							threeAddr->imm = value;
							changed = true;
						} else {
							const std::set<const IR::Entry*> &defs = useDefs.defines(entry, threeAddr->rhs2);
							if(defs.size() == 1) {
								const IR::Entry *def = *(defs.begin());
								IR::EntryThreeAddr *defThreeAddr = (IR::EntryThreeAddr*)def;
								if(def->type == IR::Entry::Type::Mult && !defThreeAddr->rhs2 && isPowerOfTwo(defThreeAddr->imm)) {
									analysis.replaceUse(threeAddr, threeAddr->rhs2, defThreeAddr->rhs1);
									threeAddr->rhs2 = defThreeAddr->rhs1;
									threeAddr->imm = log2(defThreeAddr->imm);
								}
							}
						}
						break;
					}
				case IR::Entry::Type::CJump:
					{
						// Check if the predicate of the conditional jump is constant
						IR::EntryCJump *cJump = (IR::EntryCJump*)entry;
						bool isConstant;
						int value = constants.getIntValue(cJump, cJump->pred, isConstant);
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
						procedure.entries().insert(cJump, jump);
						procedure.entries().erase(cJump);
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