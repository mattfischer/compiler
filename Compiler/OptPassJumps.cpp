#include "OptPassJumps.h"

static IR::Block *getJumpTarget(IR::Block *block)
{
	for(;;) {
		if(block->entries.size() == 0)
			break;
		
		IR::Entry *entry = block->entries[0];
		
		if(entry->type == IR::Entry::TypeJump)
			block = ((IR::Entry::Jump*)entry)->target;
		else
			break;
	}

	return block;
}

void OptPassJumps::optimize(IR *ir)
{
	for(unsigned int i=0; i<ir->procedures().size(); i++) {
		IR::Procedure *proc = ir->procedures()[i];

		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];

			if(block->entries.size() == 0)
				continue;

			IR::Entry *entry = block->entries[block->entries.size() - 1];

			if(entry->type == IR::Entry::TypeJump) {
				IR::Entry::Jump *jump = (IR::Entry::Jump*)entry;
				IR::Block *target = getJumpTarget(jump->target);

				if(target != jump->target) {
					jump->target->removePred(block);
					block->removeSucc(jump->target);
					jump->target = target;
				}
			} else if(entry->type == IR::Entry::TypeCJump) {
				IR::Entry::CJump *jump = (IR::Entry::CJump*)entry;
				IR::Block *trueTarget = getJumpTarget(jump->trueTarget);
				IR::Block *falseTarget = getJumpTarget(jump->falseTarget);

				if(trueTarget != jump->trueTarget) {
					jump->trueTarget->removePred(block);
					block->removeSucc(jump->trueTarget);

					trueTarget->addPred(block);
					block->addSucc(trueTarget);
					jump->trueTarget = trueTarget;
				}

				if(falseTarget != jump->falseTarget) {
					jump->falseTarget->removePred(block);
					block->removeSucc(jump->falseTarget);

					falseTarget->addPred(block);
					block->addSucc(falseTarget);
					jump->falseTarget = falseTarget;
				}
			}
		}
	}
}