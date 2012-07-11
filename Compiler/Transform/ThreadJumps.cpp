#include "Transform/ThreadJumps.h"

#include "IR/Block.h"
#include "IR/Entry.h"
#include "IR/Procedure.h"

namespace Transform {
	IR::Block *ThreadJumps::getJumpTarget(IR::Block *block)
	{
		for(;;) {
			IR::Entry *entry = block->entries.front();
			
			if(entry->type != IR::Entry::TypeJump)
				break;
			
			block = ((IR::EntryJump*)entry)->target;
		}

		return block;
	}

	void ThreadJumps::transform(IR::Procedure *proc)
	{
		// follow jumps
		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];
			IR::Entry *entry = block->entries.back();

			if(entry->type == IR::Entry::TypeJump) {
				IR::EntryJump *jump = (IR::EntryJump*)entry;
				IR::Block *target = getJumpTarget(jump->target);
				jump->target = target;
			} else if(entry->type == IR::Entry::TypeCJump) {
				IR::EntryCJump *jump = (IR::EntryCJump*)entry;
				IR::Block *trueTarget = getJumpTarget(jump->trueTarget);
				IR::Block *falseTarget = getJumpTarget(jump->falseTarget);
				jump->trueTarget = trueTarget;
				jump->falseTarget = falseTarget;
			}
		}
	}
}
