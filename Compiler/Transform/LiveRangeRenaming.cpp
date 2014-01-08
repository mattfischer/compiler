#include "Transform/LiveRangeRenaming.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "Analysis/UseDefs.h"

#include <sstream>
#include <queue>

namespace Transform {

void renameSymbol(IR::Entry *entry, IR::Symbol *symbol, IR::Symbol *newSymbol, Analysis::UseDefs &useDefs)
{
	std::queue<IR::Entry*> entries;

	entries.push(entry);
	while(!entries.empty()) {
		IR::Entry *entry = entries.front();
		entries.pop();

		if(entry->assign() == symbol) {
			entry->replaceAssign(symbol, newSymbol);
			const IR::EntrySet &set = useDefs.uses(entry);
			for(IR::EntrySet::const_iterator it = set.begin(); it != set.end(); it++) {
				entries.push(*it);
			}
		}

		if(entry->uses(symbol)) {
			entry->replaceUse(symbol, newSymbol);
			const IR::EntrySet &set = useDefs.defines(entry, symbol);
			for(IR::EntrySet::const_iterator it = set.begin(); it != set.end(); it++) {
				entries.push(*it);
			}
		}
	}
}

bool LiveRangeRenaming::transform(IR::Procedure *procedure)
{
	bool changed = false;

	std::vector<IR::Symbol*> newSymbols;

	Analysis::UseDefs useDefs(procedure);

	for(IR::Procedure::SymbolList::iterator symbolIt = procedure->symbols().begin(); symbolIt != procedure->symbols().end(); symbolIt++) {
		IR::Symbol *symbol = *symbolIt;
		int idx = 0;

		for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
			IR::Entry *entry = *entryIt;

			if(entry->assign() == symbol || entry->uses(symbol)) {
				std::string newName;
				if(idx == 0) {
					newName = symbol->name;
				} else {
					std::stringstream s;
					s << symbol->name << "." << idx;
					newName = s.str();
				}
				idx++;

				IR::Symbol *newSymbol = new IR::Symbol(newName, symbol->type);
				newSymbols.push_back(newSymbol);
				renameSymbol(entry, symbol, newSymbol, useDefs);
			}
		}

		if(idx > 1) {
			changed = true;
		}
	}

	procedure->symbols().clear();
	procedure->symbols().insert(procedure->symbols().begin(), newSymbols.begin(), newSymbols.end());

	return changed;
}

LiveRangeRenaming *LiveRangeRenaming::instance()
{
	static LiveRangeRenaming inst;
	return &inst;
}

}