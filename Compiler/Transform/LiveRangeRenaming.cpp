#include "Transform/LiveRangeRenaming.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "Analysis/UseDefs.h"

#include <sstream>
#include <queue>

namespace Transform {

/*!
 * \brief Rename all uses of a symbol that are connected to the given entry by use-def or def-use chains
 * \param entry Entry to scan from
 * \param symbol Symbol to rename
 * \param newSymbol Symbol to rename to
 * \param useDefs Use-def chains for the procedure
 */
void renameSymbol(IR::Entry *entry, IR::Symbol *symbol, IR::Symbol *newSymbol, Analysis::UseDefs *useDefs)
{
	std::queue<IR::Entry*> entries;

	// Begin by processing the given entry
	entries.push(entry);

	// Iterate through the queue until empty
	while(!entries.empty()) {
		IR::Entry *entry = entries.front();
		entries.pop();

		if(entry->assign() == symbol) {
			// If the entry assigns to the symbol, rename it and add all uses of the assignment
			// to the queue for further processing
			entry->replaceAssign(symbol, newSymbol);
			for(IR::Entry *use : useDefs->uses(entry)) {
				entries.push(use);
			}
		}

		if(entry->uses(symbol)) {
			// If the entry uses the symbol, rename it and add all definitions of the symbol
			// to the queue for further processing
			entry->replaceUse(symbol, newSymbol);
			for(IR::Entry *def : useDefs->defines(entry, symbol)) {
				entries.push(def);
			}
		}
	}
}

bool LiveRangeRenaming::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
{
	bool changed = false;

	std::vector<IR::Symbol*> newSymbols;

	// Construct use-def chains for the procedure
	Analysis::UseDefs *useDefs = analysis.useDefs();

	// Iterate through each symbol in the procedure
	for(IR::Symbol *symbol : procedure->symbols()) {
		int idx = 0;

		// Iterate through each entry in the procedure
		for(IR::Entry *entry : procedure->entries()) {
			// If the given symbol is assigned or used in this entry, rename all uses of the variable
			// that are connected to this one by def-use or use-def chains
			if(entry->assign() == symbol || entry->uses(symbol)) {
				std::string newName;
				if(idx == 0) {
					// Preserve the same name for the first live range
					newName = symbol->name;
				} else {
					// For all other ranges, append an integer to the symbol's name
					std::stringstream s;
					s << symbol->name << "." << idx;
					newName = s.str();
				}
				idx++;

				// Rename the symbol
				IR::Symbol *newSymbol = new IR::Symbol(newName, symbol->size, symbol->symbol);
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

	if(changed) {
		analysis.invalidate();
	}

	return changed;
}

/*!
 * \brief Singleton
 * \return Instance
 */
LiveRangeRenaming *LiveRangeRenaming::instance()
{
	static LiveRangeRenaming inst;
	return &inst;
}

}