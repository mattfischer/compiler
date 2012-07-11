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
			for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				if(entry->type != IR::Entry::TypeLoad) {
					continue;
				}

				IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
				IR::Symbol *oldSymbol = threeAddr->lhs;
				IR::Symbol *newSymbol = threeAddr->rhs1;
				IR::EntrySet oldDefs = reachingDefs.defsForSymbol(entry, newSymbol);

				IR::EntrySet uses = useDefs.uses(entry);
				for(IR::EntrySet::const_iterator it = uses.begin(); it != uses.end(); it++) {
					IR::Entry *use = *it;
					if(useDefs.defines(use, oldSymbol).size() > 1) {
						continue;
					}

					IR::EntrySet newDefs = reachingDefs.defsForSymbol(use, newSymbol);
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