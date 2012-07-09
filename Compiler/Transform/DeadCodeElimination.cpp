#include "Transform/DeadCodeElimination.h"

#include "IR/Procedure.h"
#include "IR/Block.h"
#include "IR/Entry.h"

#include "Analysis/UseDefs.h"

namespace Transform {
	void DeadCodeElimination::transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs)
	{
		for(unsigned int i=0; i<procedure->blocks().size(); i++) {
			IR::Block *block = procedure->blocks()[i];
			for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
				switch(entry->type) {
					case IR::Entry::TypeAdd:
					case IR::Entry::TypeMult:
					case IR::Entry::TypeLoad:
					case IR::Entry::TypeLoadImm:
					case IR::Entry::TypeEqual:
					case IR::Entry::TypeNequal:
						{
							const Analysis::UseDefs::EntrySet &uses = useDefs.uses(entry);
							if(uses.empty()) {
								entry->remove();
								useDefs.remove(entry);
							}
							break;
						}
				}
			}
		}
	}
}
