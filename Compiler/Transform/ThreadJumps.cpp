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

	bool ThreadJumps::transform(IR::Procedure *proc, Analysis::Analysis &analysis)
	{
		bool changed = false;

		// follow jumps
		for(IR::EntryList::iterator itEntry = proc->entries().begin(); itEntry != proc->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						IR::EntryLabel *target = getJumpTarget(jump->target);
						if(target != jump->target) {
							jump->target = target;
							changed = true;
						}
						break;
					}
				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *jump = (IR::EntryCJump*)entry;
						IR::EntryLabel *trueTarget = getJumpTarget(jump->trueTarget);
						IR::EntryLabel *falseTarget = getJumpTarget(jump->falseTarget);
						if(trueTarget != jump->trueTarget) {
							jump->trueTarget = trueTarget;
							changed = true;
						}

						if(falseTarget != jump->falseTarget) {
							jump->falseTarget = falseTarget;
							changed = true;
						}
						break;
					}
			}
		}

		return changed;
	}

	ThreadJumps *ThreadJumps::instance()
	{
		static ThreadJumps inst;
		return &inst;
	}
}
