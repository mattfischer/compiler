#include "OptPassDce.h"

void OptPassDce::optimizeProcedure(IR::Procedure *proc)
{
	bool changed;

	// Remove unreferenced blocks
	do {
		changed = false;
		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];

			if(block != proc->start() && block->pred.size() == 0) {
				for(unsigned int k=0; k<block->succ.size(); k++) {
					block->succ[k]->removePred(block);
				}
				proc->removeBlock(block);
				changed = true;
			}
		}
	} while(changed);

	// Remove unused variables
	do {
		changed = false;
		for(unsigned int i=0; i<proc->symbols().size(); i++) {
			IR::Symbol *symbol = proc->symbols()[i];
			
			if(symbol->uses == 0) {
				bool found = false;
				for(unsigned int j=0; j<proc->blocks().size(); j++) {
					IR::Block *block = proc->blocks()[j];
					for(unsigned int k=0; k<block->entries.size(); k++) {
						IR::Entry *entry = block->entries[k];
						if(entry->assigns(symbol)) {
							block->entries.erase(block->entries.begin() + k);
							delete entry;
							found = true;
							break;
						}
					}
					if(found)
						break;
				}
				proc->symbols().erase(proc->symbols().begin() + i);
				changed = true;
				i--;
			}
		}
	} while(changed);
}