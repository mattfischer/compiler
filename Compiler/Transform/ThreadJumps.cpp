#include "Transform/ThreadJumps.h"

#include "IR/Entry.h"
#include "IR/Procedure.h"

namespace Transform {
	/*!
	 * \brief Determine the ultimate target of a jump chain
	 * \param label Immediate jump target
	 * \return Final jump target
	 */
	IR::EntryLabel *ThreadJumps::getJumpTarget(IR::EntryLabel *label)
	{
		// Continue following jumps as long as the first entry following the
		// target label is another jump
		for(;;) {
			IR::Entry *entry = label->next;
			
			if(entry->type != IR::Entry::Type::Jump)
				break;
			
			label = ((IR::EntryJump*)entry)->target;
		}

		return label;
	}

	bool ThreadJumps::transform(IR::Procedure &proc, Analysis::Analysis &analysis)
	{
		bool changed = false;

		// Iterate through the procedure's entries
		for(IR::Entry *entry : proc.entries()) {
			switch(entry->type) {
				case IR::Entry::Type::Jump:
					{
						// Find ultimate target of the jump
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						IR::EntryLabel *target = getJumpTarget(jump->target);
						if(target != jump->target) {
							// Replace the jump's target with the new target
							jump->target = target;
							changed = true;
						}
						break;
					}
				case IR::Entry::Type::CJump:
					{
						IR::EntryCJump *jump = (IR::EntryCJump*)entry;

						// Find the ultimate target for both the true and false targets of the jump
						IR::EntryLabel *trueTarget = getJumpTarget(jump->trueTarget);
						IR::EntryLabel *falseTarget = getJumpTarget(jump->falseTarget);

						if(trueTarget != jump->trueTarget) {
							// Replace the jump's true target
							jump->trueTarget = trueTarget;
							changed = true;
						}

						if(falseTarget != jump->falseTarget) {
							// Replace the jump's false target
							jump->falseTarget = falseTarget;
							changed = true;
						}
						break;
					}
			}
		}

		if(changed) {
			analysis.invalidate();
		}

		return changed;
	}

	/*!
	 * \brief Singleton
	 * \return Instance
	 */
	ThreadJumps *ThreadJumps::instance()
	{
		static ThreadJumps inst;
		return &inst;
	}
}
