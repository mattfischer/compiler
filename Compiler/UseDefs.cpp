#include "UseDefs.h"

#include <stdio.h>

UseDefs::UseDefs(std::vector<IR::Block*> &blocks, ReachingDefs &reachingDefs)
: mBlocks(blocks)
{
	for(unsigned int i=0; i<blocks.size(); i++) {
		IR::Block *block = blocks[i];
		for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
			EntrySet defs = reachingDefs.defs(entry);
			for(EntrySet::iterator it = defs.begin(); it != defs.end(); it++) {
				IR::Entry *defEntry = *it;
				IR::Symbol *symbol = defEntry->assignSymbol();
				if(entry->uses(symbol)) {
					mDefines[entry][symbol].insert(defEntry);
					mUses[defEntry].insert(entry);
				}
			}
		}
	}
}

std::set<IR::Entry*> &UseDefs::uses(IR::Entry *define)
{
	return mUses[define];
}

std::set<IR::Entry*> &UseDefs::defines(IR::Entry *use, IR::Symbol *symbol)
{
	return mDefines[use][symbol];
}

void UseDefs::print()
{
	int line = 1;
	std::map<IR::Entry*, int> lineMap;
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		IR::Block *block = mBlocks[i];
		for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
			lineMap[entry] = line;
			line++;
		}
	}

	for(unsigned int i=0; i<mBlocks.size(); i++) {
		IR::Block *block = mBlocks[i];
		for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
			printf("%i: ", lineMap[entry]);
			entry->print();

			printf(" || Uses: ");
			EntrySet &uses = mUses[entry];
			for(EntrySet::iterator it = uses.begin(); it != uses.end(); it++) {
				IR::Entry *e = *it;
				printf("%i ", lineMap[e]);
			}

			SymbolToEntrySetMap &defs = mDefines[entry];
			for(SymbolToEntrySetMap::iterator it = defs.begin(); it != defs.end(); it++) {
				printf(" | Defs (%s): ", it->first->name.c_str());
				for(EntrySet::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
					IR::Entry *e = *it2;
					printf("%i ", lineMap[e]);
				}
			}

			printf("\n");
		}
	}
}
