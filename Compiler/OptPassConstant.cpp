#include "OptPassConstant.h"

void OptPassConstant::optimizeProcedure(IR::Procedure *proc)
{
	bool changed;
	do {
		changed = false;

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
	} while(changed);
}
			