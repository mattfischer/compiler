#include "OptPassConstant.h"

bool OptPassConstant::optimizeProcedure(IR::Procedure *proc)
{
	bool changed = false;

	for(unsigned int i=0; i<proc->symbols().size(); i++) {
		IR::Symbol *symbol = proc->symbols()[i];
		if(symbol->assigns.size() == 0)
			continue;

		IR::Entry *assign = symbol->assigns[0];
		bool constant;
		int value = assign->value(constant);

		if(constant) {
			if(!symbol->value || *symbol->value != value) {
				IR::Entry *imm = new IR::EntryImm(IR::Entry::TypeLoadImm, symbol, value);
				assign->replace(imm);
				delete assign;

				delete symbol->value;
				symbol->value = new int;
				*symbol->value = value;

				changed = true;
			}
		}
	}

	for(unsigned int i=0; i<proc->blocks().size(); i++) {
		IR::Block *block = proc->blocks()[i];
		IR::Entry *entry = block->tail()->prev;

		if(entry->type != IR::Entry::TypeCJump) {
			continue;
		}

		IR::EntryCJump *cjump = (IR::EntryCJump*)entry;
		IR::Symbol *symbol = cjump->pred;

		if(symbol->assigns.size() == 0) {
			continue;
		}

		bool constant;
		int value = symbol->assigns[0]->value(constant);

		if(!constant) {
			continue;
		}
			
		if(value == 0) {
			cjump->trueTarget->removePred(block);
			block->removeSucc(cjump->trueTarget);
			cjump->replace(new IR::EntryJump(cjump->falseTarget));
		} else {
			cjump->falseTarget->removePred(block);
			block->removeSucc(cjump->falseTarget);
			cjump->replace(new IR::EntryJump(cjump->trueTarget));
		}

		delete cjump;
	}

	return changed;
}
			