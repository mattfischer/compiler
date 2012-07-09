#include "Transform/CopyProp.h"

#include "IR/Procedure.h"
#include "IR/Block.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "Analysis/UseDefs.h"
#include "Analysis/ReachingDefs.h"

namespace Transform {
	void CopyProp::transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs, Analysis::ReachingDefs &reachingDefs)
	{
		for(unsigned int i=0; i<procedure->blocks().size(); i++) {
			IR::Block *block = procedure->blocks()[i];
			for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
				if(entry->type != IR::Entry::TypeLoad) {
					continue;
				}

				IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
				IR::Symbol *oldSymbol = threeAddr->lhs;
				IR::Symbol *newSymbol = threeAddr->rhs1;
				Analysis::ReachingDefs::EntrySet oldDefs = reachingDefs.defsForSymbol(entry, newSymbol);

				Analysis::UseDefs::EntrySet uses = useDefs.uses(entry);
				for(Analysis::UseDefs::EntrySet::const_iterator it = uses.begin(); it != uses.end(); it++) {
					IR::Entry *use = *it;
					if(useDefs.defines(use, oldSymbol).size() > 1) {
						continue;
					}

					Analysis::ReachingDefs::EntrySet newDefs = reachingDefs.defsForSymbol(use, newSymbol);
					if(oldDefs != newDefs) {
						continue;
					}

					use->replaceUse(oldSymbol, newSymbol);
					useDefs.replaceUse(use, oldSymbol, newSymbol);
				}
			}
		}
	}
}