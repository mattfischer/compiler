#include "Transform/ThreadJumps.h"

#include "IR/Block.h"
#include "IR/Entry.h"
#include "IR/Procedure.h"

namespace Transform {
	IR::EntryLabel *ThreadJumps::getJumpTarget(IR::EntryLabel *label)
	{
		for(;;) {
			IR::Entry *entry = label->next;
			
			if(entry->type != IR::Entry::TypeJump)
				break;
			
			label = ((IR::EntryJump*)entry)->target;
		}

		return label;
	}

	void ThreadJumps::transform(IR::Procedure *proc)
	{
		// follow jumps
		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];
			IR::Entry *entry = block->entries.back();

			if(entry->type == IR::Entry::TypeJump) {
				IR::EntryJump *jump = (IR::EntryJump*)entry;
				IR::EntryLabel *target = getJumpTarget(jump->target);
				jump->target = target;
			} else if(entry->type == IR::Entry::TypeCJump) {
				IR::EntryCJump *jump = (IR::EntryCJump*)entry;
				IR::EntryLabel *trueTarget = getJumpTarget(jump->trueTarget);
				IR::EntryLabel *falseTarget = getJumpTarget(jump->falseTarget);
				jump->trueTarget = trueTarget;
				jump->falseTarget = falseTarget;
			}
		}
	}
}
