#include "OptPassDce.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Block.h"

bool OptPassDce::optimizeProcedure(IR::Procedure *proc)
{
	bool changed = false;
	
	// Remove unreferenced blocks
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

	// Remove unused variables
	for(unsigned int i=0; i<proc->symbols().size(); i++) {
		IR::Symbol *symbol = proc->symbols()[i];
		
		if(symbol->uses == 0) {
			while(!symbol->assigns.empty()) {
				IR::Entry *entry = symbol->assigns.front();

				entry->remove();
				delete entry;
			}

			proc->symbols().erase(proc->symbols().begin() + i);
			changed = true;
			i--;
		}
	}

	return changed;
}