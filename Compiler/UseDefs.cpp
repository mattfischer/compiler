#include "UseDefs.h"

#include <stdio.h>

static UseDefs::EntrySet emptyEntrySet;

UseDefs::UseDefs(const std::vector<IR::Block*> &blocks, const ReachingDefs &reachingDefs)
: mBlocks(blocks)
{
	for(unsigned int i=0; i<blocks.size(); i++) {
		IR::Block *block = blocks[i];
		for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
			const EntrySet &defs = reachingDefs.defs(entry);
			for(EntrySet::const_iterator it = defs.begin(); it != defs.end(); it++) {
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

const UseDefs::EntrySet &UseDefs::uses(IR::Entry *define) const
{
	std::map<IR::Entry*, EntrySet>::const_iterator it = mUses.find(define);
	if(it != mUses.end()) {
		return it->second;
	} else {
		return emptyEntrySet;
	}
}

const std::set<IR::Entry*> &UseDefs::defines(IR::Entry *use, IR::Symbol *symbol) const
{
	std::map<IR::Entry*, SymbolToEntrySetMap>::const_iterator it = mDefines.find(use);
	if(it != mDefines.end()) {
		const SymbolToEntrySetMap &map = it->second;
		SymbolToEntrySetMap::const_iterator it2 = map.find(symbol);
		if(it2 != map.end()) {
			return it2->second;
		} else {
			return emptyEntrySet;
		}
	} else {
		return emptyEntrySet;
	}
}

void UseDefs::print() const
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
			const EntrySet &u = mUses.find(entry)->second;
			for(EntrySet::const_iterator it = u.begin(); it != u.end(); it++) {
				IR::Entry *e = *it;
				printf("%i ", lineMap[e]);
			}

			const SymbolToEntrySetMap &defs = mDefines.find(entry)->second;
			for(SymbolToEntrySetMap::const_iterator it = defs.begin(); it != defs.end(); it++) {
				printf(" | Defs (%s): ", it->first->name.c_str());
				for(EntrySet::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
					IR::Entry *e = *it2;
					printf("%i ", lineMap[e]);
				}
			}

			printf("\n");
		}
	}
}
