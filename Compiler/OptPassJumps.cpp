#include "OptPassJumps.h"

static IR::Block *getJumpTarget(IR::Block *block)
{
	for(;;) {
		IR::Entry *entry = block->head()->next;
		
		if(entry->type != IR::Entry::TypeJump)
			break;
		
		block = ((IR::EntryJump*)entry)->target;
	}

	return block;
}

bool OptPassJumps::optimizeProcedure(IR::Procedure *proc)
{
	bool changed = false;

	// follow jumps
	for(unsigned int j=0; j<proc->blocks().size(); j++) {
		IR::Block *block = proc->blocks()[j];
		IR::Entry *entry = block->tail()->prev;

		if(entry->type == IR::Entry::TypeJump) {
			IR::EntryJump *jump = (IR::EntryJump*)entry;
			IR::Block *target = getJumpTarget(jump->target);

			if(target != jump->target) {
				jump->target->removePred(block);
				block->removeSucc(jump->target);

				jump->target->addPred(block);
				block->addSucc(jump->target);
				jump->target = target;
				changed = true;
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

				changed = true;
			}

			if(falseTarget != jump->falseTarget) {
				jump->falseTarget->removePred(block);
				block->removeSucc(jump->falseTarget);

				falseTarget->addPred(block);
				block->addSucc(falseTarget);
				jump->falseTarget = falseTarget;

				changed = true;
			}
		}
	}

	// Combine sequential blocks
	for(unsigned int j=0; j<proc->blocks().size(); j++) {
		IR::Block *block = proc->blocks()[j];
		if(block->succ.size() != 1 || block->succ[0]->pred.size() != 1) {
			continue;
		}
		IR::Block *succ = block->succ[0];

		for(IR::Entry *entry = succ->head()->next; entry != succ->tail(); entry = entry->next) {
			if(entry->type == IR::Entry::TypePhi) {
				IR::EntryPhi *phi = (IR::EntryPhi*)entry;
				phi->replace(new IR::EntryThreeAddr(IR::Entry::TypeLoad, phi->lhs, phi->args[0]));
				entry = entry->prev;
				delete phi;
			}
		}

		block->tail()->prev->remove();

		block->tail()->prev->next = succ->head()->next;
		succ->head()->next->prev = block->tail()->prev;
		block->tail()->prev = succ->tail()->prev;
		succ->tail()->prev->next = block->tail();
		
		succ->head()->next = succ->tail();
		succ->tail()->prev = succ->head();

		for(int k=0; k<succ->succ.size(); k++) {
			IR::Block *succSucc = succ->succ[k];
			succSucc->replacePred(succ, block);
			succ->removeSucc(succSucc);
			block->addSucc(succSucc);
		}

		block->removeSucc(succ);
		succ->removePred(block);

		if(succ == proc->end()) {
			proc->replaceEnd(block);
		}

		changed = true;
		j--;
	}

	return changed;
}