#include "Transform/ConstantProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"

#include "Util/UniqueQueue.h"

namespace Transform {
	int ConstantProp::getValue(IR::Entry *entry, IR::Symbol *symbol, Analysis::UseDefs &useDefs, bool &isConstant)
	{
		isConstant = false;
		int ret = 0;

		const IR::EntrySet &set = useDefs.defines(entry, symbol);
		for(IR::EntrySet::const_iterator it = set.begin(); it != set.end(); it++) {
			IR::Entry *def = *it;
			if(def->type != IR::Entry::TypeLoadImm) {
				ret = 0;
				isConstant = false;
				break;
			}

			IR::EntryOneAddrImm *imm = (IR::EntryOneAddrImm*)def;
			if(isConstant && ret != imm->imm) {
				ret = 0;
				isConstant = false;
				break;
			} else {
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

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			queue.push(entry);
		}

		while(!queue.empty()) {
			IR::Entry *entry = queue.front();
			queue.pop();

			switch(entry->type) {
				case IR::Entry::TypeAdd:
				case IR::Entry::TypeMult:
				case IR::Entry::TypeLoad:
				case IR::Entry::TypeEqual:
				case IR::Entry::TypeNequal:
					{
						int rhs1;
						int rhs2;
						bool rhs1Const;
						bool rhs2Const;

						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						rhs1 = getValue(threeAddr, threeAddr->rhs1, useDefs, rhs1Const);
						if(threeAddr->rhs2) {
							rhs2 = getValue(threeAddr, threeAddr->rhs2, useDefs, rhs2Const);
						} else {
							rhs2 = 0;
							rhs2Const = true;
						}

						IR::Entry *newEntry = 0;
						if(rhs1Const && rhs2Const) {
							int value;
							switch(entry->type) {
								case IR::Entry::TypeAdd:
									value = rhs1 + rhs2;
									break;

								case IR::Entry::TypeMult:
									value = rhs1 * rhs2;
									break;

								case IR::Entry::TypeLoad:
									value = rhs1;
									break;

								case IR::Entry::TypeEqual:
									value = rhs1 == rhs2;
									break;

								case IR::Entry::TypeNequal:
									value = rhs1 != rhs2;
									break;
							}

							newEntry = new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, threeAddr->lhs, value);
						} else if(rhs1Const || rhs2Const) {
							if(threeAddr->type == IR::Entry::TypeAdd) {
								int constant;
								IR::Symbol *symbol;
								if(rhs1Const) {
									constant = rhs1;
									symbol = threeAddr->rhs2;
								} else {
									constant = rhs2;
									symbol = threeAddr->rhs1;
								}

								newEntry = new IR::EntryTwoAddrImm(IR::Entry::TypeAddImm, threeAddr->lhs, symbol, constant);
							}
						}

						if(newEntry) {
							const IR::EntrySet &entries = useDefs.uses(threeAddr);
							for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
								queue.push(*it);
							}

							procedure->entries().insert(threeAddr, newEntry);
							procedure->entries().erase(threeAddr);
							delete threeAddr;
							changed = true;
						}
						break;
					}

				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *cJump = (IR::EntryCJump*)entry;
						bool isConstant;
						int value = getValue(cJump, cJump->pred, useDefs, isConstant);
						if(!isConstant) {
							continue;
						}

						IR::EntryJump *jump;
						if(value) {
							jump = new IR::EntryJump(cJump->trueTarget);
						} else {
							jump = new IR::EntryJump(cJump->falseTarget);
						}

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

	ConstantProp *ConstantProp::instance()
	{
		static ConstantProp inst;
		return &inst;
	}
}