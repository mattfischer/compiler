#include "Transform/ConstantProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/Analysis.h"

#include <queue>
#include <map>

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

			IR::EntryImm *imm = (IR::EntryImm*)def;
			if(isConstant && ret != imm->rhs) {
				ret = 0;
				isConstant = false;
				break;
			} else {
				ret = imm->rhs;
				isConstant = true;
			}
		}

		return ret;
	}

	void ConstantProp::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		std::queue<IR::Entry*> queue;

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
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						bool isConstant;
						int rhs1 = getValue(threeAddr, threeAddr->rhs1, analysis.useDefs(), isConstant);
						if(!isConstant) {
							continue;
						}

						int rhs2 = 0;
						if(threeAddr->rhs2) {
							rhs2 = getValue(threeAddr, threeAddr->rhs2, analysis.useDefs(), isConstant);
							if(!isConstant) {
								continue;
							}
						}

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

						IR::Entry *imm = new IR::EntryImm(threeAddr->lhs, value);
						procedure->entries().insert(threeAddr, imm);
						procedure->entries().erase(threeAddr);

						const IR::EntrySet &entries = analysis.useDefs().uses(threeAddr);
						for(IR::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
							queue.push(*it);
						}

						analysis.replace(threeAddr, imm);
						delete threeAddr;
						break;
					}

				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *cJump = (IR::EntryCJump*)entry;
						bool isConstant;
						int value = getValue(cJump, cJump->pred, analysis.useDefs(), isConstant);
						if(!isConstant) {
							continue;
						}

						IR::EntryJump *jump;
						if(value) {
							jump = new IR::EntryJump(cJump->trueTarget);
						} else {
							jump = new IR::EntryJump(cJump->falseTarget);
						}

						analysis.replace(cJump, jump);

						procedure->entries().insert(cJump, jump);
						procedure->entries().erase(cJump);

						delete cJump;
						break;
					}
			}
		}
	}
}