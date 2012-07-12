#include "Transform/ThreadJumps.h"

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
		for(IR::EntryList::iterator itEntry = proc->entries().begin(); itEntry != proc->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						IR::EntryLabel *target = getJumpTarget(jump->target);
						jump->target = target;
						break;
					}
				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *jump = (IR::EntryCJump*)entry;
						IR::EntryLabel *trueTarget = getJumpTarget(jump->trueTarget);
						IR::EntryLabel *falseTarget = getJumpTarget(jump->falseTarget);
						jump->trueTarget = trueTarget;
						jump->falseTarget = falseTarget;
						break;
					}
			}
		}
	}
}
