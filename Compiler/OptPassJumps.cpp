#include "OptPassJumps.h"

static IR::Block *getJumpTarget(IR::Block *block)
{
	for(;;) {
		IR::Entry *entry = block->head();
		
		if(!entry || entry->type != IR::Entry::TypeJump)
			break;
		
		block = ((IR::EntryJump*)entry)->target;
	}

	return block;
}

void OptPassJumps::optimizeProcedure(IR::Procedure *proc)
{
	for(unsigned int j=0; j<proc->blocks().size(); j++) {
		IR::Block *block = proc->blocks()[j];
		IR::Entry *entry = block->tail();

		if(!entry)
			continue;

		if(entry->type == IR::Entry::TypeJump) {
			IR::EntryJump *jump = (IR::EntryJump*)entry;
			IR::Block *target = getJumpTarget(jump->target);

			if(target != jump->target) {
				jump->target->removePred(block);
				block->removeSucc(jump->target);
				jump->target = target;
			}
		} else if(entry->type == IR::Entry::TypeCJump) {
			IR::EntryCJump *jump = (IR::EntryCJump*)entry;
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