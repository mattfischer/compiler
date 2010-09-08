#include "OptPassCopy.h"

void OptPassCopy::optimizeProcedure(IR::Procedure *proc)
{
	bool changed;
	do {
		changed = false;

		for(unsigned int i=0; i<proc->symbols().size(); i++) {
			IR::Symbol *symbol = proc->symbols()[i];

			if(symbol->assigns.size() == 0)
				continue;

			IR::Entry *assign = symbol->assigns[0];

			if(assign->type == IR::Entry::TypeLoad) {
				IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)assign;
				IR::Symbol *parent = threeAddr->rhs1;
				IR::Entry *parentAssign = parent->assigns[0];

				for(unsigned int j=0; j<proc->blocks().size(); j++) {
					IR::Block *block = proc->blocks()[j];

					for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
						if(entry->uses(parent))
							entry->replaceUse(parent, symbol);
					}
				}

				parentAssign->replaceAssign(parent, symbol);
				assign->remove();
				delete assign;

				changed = true;
			}
		}
	} while(changed);
}