#include "Transform/CopyProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "Analysis/Analysis.h"
#include "Analysis/UseDefs.h"
#include "Analysis/ReachingDefs.h"

namespace Transform {
	bool CopyProp::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			if(entry->type != IR::Entry::TypeLoad) {
				continue;
			}

			IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
			IR::Symbol *oldSymbol = threeAddr->lhs;
			IR::Symbol *newSymbol = threeAddr->rhs1;
			IR::EntrySet oldDefs = analysis.reachingDefs().defsForSymbol(entry, newSymbol);

			IR::EntrySet uses = analysis.useDefs().uses(entry);
			for(IR::EntrySet::const_iterator it = uses.begin(); it != uses.end(); it++) {
				IR::Entry *use = *it;
				if(analysis.useDefs().defines(use, oldSymbol).size() > 1) {
					continue;
				}

				IR::EntrySet newDefs = analysis.reachingDefs().defsForSymbol(use, newSymbol);
				if(oldDefs != newDefs) {
					continue;
				}

				use->replaceUse(oldSymbol, newSymbol);
				analysis.replaceUse(use, oldSymbol, newSymbol);
				changed = true;
			}
		}

		return changed;
	}

	CopyProp *CopyProp::instance()
	{
		static CopyProp inst;
		return &inst;
	}
}