#include "UseDefs.h"

#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/ReachingDefs.h"

namespace Analysis {
	static IR::EntrySet emptyEntrySet; //!< Empty entry set, used when entry lookup fails

	/*!
	 * \brief Constructor
	 * \param procedure Procedur to analyze
	 */
	UseDefs::UseDefs(IR::Procedure *procedure)
	: mProcedure(procedure), mReachingDefs(procedure)
	{
		// Iterate through the procedure, examining the reaching definition information at each entry
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			// Iterate through the definitions which reach this entry
			const IR::EntrySet &defs = mReachingDefs.defs(entry);
			for(IR::EntrySet::const_iterator it = defs.begin(); it != defs.end(); it++) {
				IR::Entry *defEntry = *it;
				IR::Symbol *symbol = defEntry->assign();
				if(entry->uses(symbol)) {
					// If the entry uses the symbol from this definition, then add it to the
					// appropriate use-def and def-use chains
					mDefines[entry][symbol].insert(defEntry);
					mUses[defEntry].insert(entry);
				}
			}
		}
	}

	/*!
	 * \brief Return all uses of a given definition
	 * \param define Entry to search for uses of
	 * \return All entries which use the given definition
	 */
	const IR::EntrySet &UseDefs::uses(IR::Entry *define) const
	{
		std::map<IR::Entry*, IR::EntrySet>::const_iterator it = mUses.find(define);
		if(it != mUses.end()) {
			return it->second;
		} else {
			return emptyEntrySet;
		}
	}

	/*!
	 * \brief Return all defintions that reach a given use
	 * \param use Entry to search for definitions of
	 * \param symbol Symbol to search for definitions of
	 * \return All definitions of the given symbol which reach the given entry
	 */
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

	/*!
	 * \brief Replace an entry with a new entry
	 * \param oldEntry Existing entry
	 * \param newEntry New entry
	 */
	void UseDefs::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		// Remove oldEntry from the def-use chains
		SymbolToEntrySetMap &map = mDefines[oldEntry];
		for(SymbolToEntrySetMap::iterator it = map.begin(); it != map.end(); it++) {
			IR::EntrySet &defs = it->second;
			for(IR::EntrySet::iterator it2 = defs.begin(); it2 != defs.end(); it2++) {
				IR::Entry *use = *it2;
				mUses[use].erase(oldEntry);
			}
		}
		// Remove oldEntry from the use-def chains
		mDefines.erase(oldEntry);

		// Construct new def-use information for the new entry from the reaching def information
		const IR::EntrySet &newDefs = mReachingDefs.defs(oldEntry);
		for(IR::EntrySet::const_iterator it = newDefs.begin(); it != newDefs.end(); it++) {
			IR::Entry *def = *it;
			IR::Symbol *symbol = def->assign();
			if(newEntry->uses(symbol)) {
				mDefines[newEntry][symbol].insert(def);
			}
		}

		// Transfer use-def information from the old entry into the new entry
		mUses[newEntry] = mUses[oldEntry];
		mUses.erase(oldEntry);
		IR::EntrySet &uses = mUses[newEntry];
		for(IR::EntrySet::iterator it = uses.begin(); it != uses.end(); it++) {
			IR::Entry *use = *it;
			IR::EntrySet &defs = mDefines[use][newEntry->assign()];
			defs.erase(oldEntry);
			defs.insert(newEntry);
		}

		// Replace the entry in the reaching def information
		mReachingDefs.replace(oldEntry, newEntry);
	}

	/*!
	 * \brief Remove an entry from the chains
	 * \param entry Entry to remove
	 */
	void UseDefs::remove(IR::Entry *entry)
	{
		// If the entry both uses and defines the same symbol, use-def information
		// needs to be propagated downward from the entry's own use-def chains
		if(entry->uses(entry->assign())) {
			IR::Symbol *symbol = entry->assign();
			IR::EntrySet &defs = mDefines[entry][symbol];
			IR::EntrySet &uses = mUses[entry];
			for(IR::EntrySet::iterator itUse = uses.begin(); itUse != uses.end(); itUse++) {
				IR::Entry *use = *itUse;
				for(IR::EntrySet::iterator itDef = defs.begin(); itDef != defs.end(); itDef++) {
					IR::Entry *def = *itDef;
					mDefines[use][symbol].insert(def);
					mUses[def].insert(use);
				}
			}
		}

		// Remove the entry from the use-def chains
		SymbolToEntrySetMap &map = mDefines[entry];
		for(SymbolToEntrySetMap::iterator it = map.begin(); it != map.end(); it++) {
			IR::EntrySet &defSet = it->second;
			for(IR::EntrySet::iterator it2 = defSet.begin(); it2 != defSet.end(); it2++) {
				IR::Entry *def = *it2;
				mUses[def].erase(entry);
			}
		}

		// Remove the entry from the def-use chains
		mDefines.erase(entry);
		IR::EntrySet &uses = mUses[entry];
		for(IR::EntrySet::iterator it = uses.begin(); it != uses.end(); it++) {
			IR::Entry *use = *it;
			mDefines[use][entry->assign()].erase(entry);
		}
		mUses.erase(entry);

		// Remove the entry from the reaching def information
		mReachingDefs.remove(entry);
	}

	/*!
	 * \brief Replace the use of a symbol with another symbol
	 * \param entry Entry to examine
	 * \param oldSymbol Existing symbol
	 * \param newSymbol New symbol
	 */
	void UseDefs::replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol)
	{
		// Remove the existing def-use and use-def information
		IR::EntrySet &oldDefs = mDefines[entry][oldSymbol];
		for(IR::EntrySet::iterator it = oldDefs.begin(); it != oldDefs.end(); it++) {
			IR::Entry *def = *it;
			mUses[def].erase(entry);
		}
		mDefines[entry].erase(oldSymbol);

		// Construct new use-def and def-use information from the underlying reaching def information
		const IR::EntrySet &newDefs = mReachingDefs.defsForSymbol(entry, newSymbol);
		for(IR::EntrySet::const_iterator it = newDefs.begin(); it != newDefs.end(); it++) {
			IR::Entry *def = *it;
			mUses[def].insert(entry);
		}
		mDefines[entry][newSymbol] = newDefs;
	}

	/*!
	 * \brief Print out use-def and def-use information
	 */
	void UseDefs::print() const
	{
		// Assign a line number to each entry
		int line = 1;
		std::map<IR::Entry*, int> lineMap;
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			lineMap[entry] = line;
			line++;
		}

		// Iterate through the procedure, printing out each entry along with def-use and use-def information
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			std::cout << lineMap[entry] << ": " << *entry;

			// Print use information
			bool printedOpen = false;
			{
				std::map<IR::Entry*, IR::EntrySet>::const_iterator it = mUses.find(entry);
				if(it != mUses.end()) {
					const IR::EntrySet &u = it->second;
					if(!u.empty()) {
						std::cout << " [ Uses: ";
						printedOpen = true;
						for(IR::EntrySet::const_iterator it = u.begin(); it != u.end(); it++) {
							IR::Entry *e = *it;
							std::cout << lineMap[e] << " ";
						}
					}
				}
			}

			// Print def information
			{
				std::map<IR::Entry*, SymbolToEntrySetMap>::const_iterator it = mDefines.find(entry);
				if(it != mDefines.end()) {
					const SymbolToEntrySetMap &defs = it->second;
					for(SymbolToEntrySetMap::const_iterator it = defs.begin(); it != defs.end(); it++) {
						if(printedOpen) {
							std::cout << "| ";
						} else {
							std::cout << " [ ";
							printedOpen = true;
						}
						std::cout << "Defs (" << it->first->name << "): ";
						for(IR::EntrySet::const_iterator it2 = it->second.begin(); it2 != it->second.end(); it2++) {
							IR::Entry *e = *it2;
							std::cout << lineMap[e] << " ";
						}
					}
				}
			}

			if(printedOpen) {
				std::cout << "]";
			}
			std::cout << std::endl;
		}
	}
}