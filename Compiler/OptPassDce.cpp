#include "OptPassDce.h"

void OptPassDce::optimize(IR *ir)
{
	for(unsigned int i=0; i<ir->procedures().size(); i++) {
		IR::Procedure *proc = ir->procedures()[i];
		bool changed;

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
	}
}