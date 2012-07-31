#include "UseDefs.h"

#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/ReachingDefs.h"

#include <stdio.h>

namespace Analysis {
	static IR::EntrySet emptyEntrySet;

	UseDefs::UseDefs(IR::Procedure *procedure)
	: mProcedure(procedure), mReachingDefs(procedure)
	{
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			const IR::EntrySet &defs = mReachingDefs.defs(entry);
			for(IR::EntrySet::const_iterator it = defs.begin(); it != defs.end(); it++) {
				IR::Entry *defEntry = *it;
				IR::Symbol *symbol = defEntry->assign();
				if(entry->uses(symbol)) {
					mDefines[entry][symbol].insert(defEntry);
					mUses[defEntry].insert(entry);
				}
			}
		}
	}

	const IR::EntrySet &UseDefs::uses(IR::Entry *define) const
	{
		std::map<IR::Entry*, IR::EntrySet>::const_iterator it = mUses.find(define);
		if(it != mUses.end()) {
			return it->second;
		} else {
			return emptyEntrySet;
		}
	}

	const IR::EntrySet &UseDefs::defines(IR::Entry *use, IR::Symbol *symbol) const
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
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			lineMap[entry] = line;
			line++;
		}

		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			printf("%i: ", lineMap[entry]);
			entry->print();

			bool printedOpen = false;
			{
				std::map<IR::Entry*, IR::EntrySet>::const_iterator it = mUses.find(entry);
				if(it != mUses.end()) {
					const IR::EntrySet &u = it->second;
					if(!u.empty()) {
						printf(" [ Uses: ");
						printedOpen = true;
						for(IR::EntrySet::const_iterator it = u.begin(); it != u.end(); it++) {
							IR::Entry *e = *it;
							printf("%i ", lineMap[e]);
						}
					}
				}
			}

			{
				std::map<IR::Entry*, SymbolToEntrySetMap>::const_iterator it = mDefines.find(entry);
				if(it != mDefines.end()) {
					const SymbolToEntrySetMap &defs = it->second;
					for(SymbolToEntrySetMap::const_iterator it = defs.begin(); it != defs.end(); it++) {
						if(printedOpen) {
							printf("| ");
						} else {
							printf(" [ ");
							printedOpen = true;
						}
						printf("Defs (%s): ", it->first->name.c_str());
						for(IR::EntrySet::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
							IR::Entry *e = *it2;
							printf("%i ", lineMap[e]);
						}
					}
				}
			}

			if(printedOpen) {
				printf("]");
			}
			printf("\n");
		}
	}
}