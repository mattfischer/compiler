#include "Transform/ConstantProp.h"

#include "IR/Procedure.h"
#include "IR/Block.h"
#include "IR/Entry.h"

#include "Analysis/UseDefs.h"
#include "Analysis/ReachingDefs.h"
#include "Analysis/FlowGraph.h"

#include <queue>
#include <map>

namespace Transform {
	int ConstantProp::getValue(IR::Entry *entry, IR::Symbol *symbol, const Analysis::UseDefs &useDefs, bool &isConstant)
	{
		isConstant = false;
		int ret = 0;

		const Analysis::UseDefs::EntrySet &set = useDefs.defines(entry, symbol);
		for(Analysis::UseDefs::EntrySet::const_iterator it = set.begin(); it != set.end(); it++) {
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

	void ConstantProp::transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs, Analysis::ReachingDefs &reachingDefs, Analysis::FlowGraph &flowGraph)
	{
		std::queue<IR::Entry*> queue;
		std::map<IR::Entry*, IR::Block*> blockMap;

		for(unsigned int i=0; i<procedure->blocks().size(); i++) {
			IR::Block *block = procedure->blocks()[i];

			for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				queue.push(entry);
				blockMap[entry] = block;
			}
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
						int rhs1 = getValue(threeAddr, threeAddr->rhs1, useDefs, isConstant);
						if(!isConstant) {
							continue;
						}

						int rhs2 = 0;
						if(threeAddr->rhs2) {
							rhs2 = getValue(threeAddr, threeAddr->rhs2, useDefs, isConstant);
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
						IR::Block *block = blockMap[threeAddr];
						block->entries.insert(threeAddr, imm);
						block->entries.erase(threeAddr);

						const Analysis::UseDefs::EntrySet &entries = useDefs.uses(threeAddr);
						for(Analysis::UseDefs::EntrySet::const_iterator it = entries.begin(); it != entries.end(); it++) {
							queue.push(*it);
						}

						useDefs.replace(threeAddr, imm);
						reachingDefs.replace(threeAddr, imm);
						delete threeAddr;
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

						IR::Block *block = blockMap[cJump];

						useDefs.replace(cJump, jump);
						reachingDefs.replace(cJump, jump);
						flowGraph.replace(cJump, jump);

						block->entries.insert(cJump, jump);
						block->entries.erase(cJump);

						delete cJump;
						break;
					}
			}
		}
	}
}