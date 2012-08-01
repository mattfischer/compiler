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

	void UseDefs::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		SymbolToEntrySetMap &map = mDefines[oldEntry];
		for(SymbolToEntrySetMap::iterator it = map.begin(); it != map.end(); it++) {
			IR::EntrySet &defs = it->second;
			for(IR::EntrySet::iterator it2 = defs.begin(); it2 != defs.end(); it2++) {
				IR::Entry *use = *it2;
				mUses[use].erase(oldEntry);
			}
		}
		mDefines.erase(oldEntry);
		const IR::EntrySet &newDefs = mReachingDefs.defs(oldEntry);
		for(IR::EntrySet::const_iterator it = newDefs.begin(); it != newDefs.end(); it++) {
			IR::Entry *def = *it;
			IR::Symbol *symbol = def->assign();
			if(newEntry->uses(symbol)) {
				mDefines[newEntry][symbol].insert(def);
			}
		}

		mUses[newEntry] = mUses[oldEntry];
		mUses.erase(oldEntry);
		IR::EntrySet &uses = mUses[newEntry];
		for(IR::EntrySet::iterator it = uses.begin(); it != uses.end(); it++) {
			IR::Entry *use = *it;
			IR::EntrySet &defs = mDefines[use][newEntry->assign()];
			defs.erase(oldEntry);
			defs.insert(newEntry);
		}

		mReachingDefs.replace(oldEntry, newEntry);
	}

	void UseDefs::remove(IR::Entry *entry)
	{
		SymbolToEntrySetMap &map = mDefines[entry];
		for(SymbolToEntrySetMap::iterator it = map.begin(); it != map.end(); it++) {
			IR::EntrySet &defSet = it->second;
			for(IR::EntrySet::iterator it2 = defSet.begin(); it2 != defSet.end(); it2++) {
				IR::Entry *def = *it2;
				mUses[def].erase(entry);
			}
		}
		mDefines.erase(entry);

		IR::EntrySet &uses = mUses[entry];
		for(IR::EntrySet::iterator it = uses.begin(); it != uses.end(); it++) {
			IR::Entry *use = *it;
			mDefines[use][entry->assign()].erase(entry);
		}
		mUses.erase(entry);

		mReachingDefs.remove(entry);
	}

	void UseDefs::replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol)
	{
		IR::EntrySet &oldDefs = mDefines[entry][oldSymbol];
		for(IR::EntrySet::iterator it = oldDefs.begin(); it != oldDefs.end(); it++) {
			IR::Entry *def = *it;
			mUses[def].erase(entry);
		}
		mDefines[entry].erase(oldSymbol);

		const IR::EntrySet &newDefs = mReachingDefs.defsForSymbol(entry, newSymbol);
		for(IR::EntrySet::const_iterator it = newDefs.begin(); it != newDefs.end(); it++) {
			IR::Entry *def = *it;
			mUses[def].insert(entry);
		}
		mDefines[entry][newSymbol] = newDefs;
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