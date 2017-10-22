#include "UseDefs.h"

#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/ReachingDefs.h"

#include "Util/Timer.h"
#include "Util/Log.h"

namespace Analysis {
	static std::set<const IR::Entry*> emptyEntrySet; //!< Empty entry set, used when entry lookup fails

	/*!
	 * \brief Constructor
	 * \param procedure Procedur to analyze
	 */
	UseDefs::UseDefs(const IR::Procedure &procedure, const ReachingDefs &reachingDefs)
	: mProcedure(procedure), mReachingDefs(reachingDefs)
	{
		Util::Timer timer;
		timer.start();

		// Iterate through the procedure, examining the reaching definition information at each entry
		for(const IR::Entry *entry : mProcedure.entries()) {
			// Iterate through the definitions which reach this entry
			const std::set<const IR::Entry*> &defs = mReachingDefs.defs(entry);
			for(const IR::Entry *defEntry : defs) {
				const IR::Symbol *symbol = defEntry->assign();
				if(entry->uses(symbol)) {
					// If the entry uses the symbol from this definition, then add it to the
					// appropriate use-def and def-use chains
					mDefines[entry][symbol].insert(defEntry);
					mUses[defEntry].insert(entry);
				}
			}
		}
		Util::log("opt.time") << "  UseDefs(" << procedure.name() << "): " << timer.stop() << "ms" << std::endl;
	}

	/*!
	 * \brief Return all uses of a given definition
	 * \param define Entry to search for uses of
	 * \return All entries which use the given definition
	 */
	const std::set<const IR::Entry*> &UseDefs::uses(const IR::Entry *define) const
	{
		auto it = mUses.find(define);
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
	const std::set<const IR::Entry*> &UseDefs::defines(const IR::Entry *use, const IR::Symbol *symbol) const
	{
		auto it = mDefines.find(use);
		if(it != mDefines.end()) {
			const std::map<const IR::Symbol*, std::set<const IR::Entry*>> &map = it->second;
			auto it2 = map.find(symbol);
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
	void UseDefs::replace(const IR::Entry *oldEntry, const IR::Entry *newEntry)
	{
		// Remove oldEntry from the def-use chains
		std::map<const IR::Symbol*, std::set<const IR::Entry*>> &map = mDefines[oldEntry];
		for(auto &def : map) {
			std::set<const IR::Entry*> &defs = def.second;
			for(const IR::Entry *use : defs) {
				mUses[use].erase(oldEntry);
			}
		}
		// Remove oldEntry from the use-def chains
		mDefines.erase(oldEntry);

		// Construct new def-use information for the new entry from the reaching def information
		const std::set<const IR::Entry*> &newDefs = mReachingDefs.defs(oldEntry);
		for(const IR::Entry *def : newDefs) {
			const IR::Symbol *symbol = def->assign();
			if(newEntry->uses(symbol)) {
				mDefines[newEntry][symbol].insert(def);
				mUses[def].insert(newEntry);
			}
		}

		// Transfer use-def information from the old entry into the new entry
		mUses[newEntry] = mUses[oldEntry];
		mUses.erase(oldEntry);
		std::set<const IR::Entry*> &uses = mUses[newEntry];
		for(const IR::Entry *use : uses) {
			std::set<const IR::Entry*> &defs = mDefines[use][newEntry->assign()];
			defs.erase(oldEntry);
			defs.insert(newEntry);
		}
	}

	/*!
	 * \brief Remove an entry from the chains
	 * \param entry Entry to remove
	 */
	void UseDefs::remove(const IR::Entry *entry)
	{
		// If the entry both uses and defines the same symbol, use-def information
		// needs to be propagated downward from the entry's own use-def chains
		if(entry->uses(entry->assign())) {
			const IR::Symbol *symbol = entry->assign();
			std::set<const IR::Entry*> &defs = mDefines[entry][symbol];
			std::set<const IR::Entry*> &uses = mUses[entry];
			for(const IR::Entry *use : uses) {
				for(const IR::Entry *def : defs) {
					mDefines[use][symbol].insert(def);
					mUses[def].insert(use);
				}
			}
		}

		// Remove the entry from the use-def chains
		std::map<const IR::Symbol*, std::set<const IR::Entry*>> &map = mDefines[entry];
		for(auto &defs : map) {
			for(const IR::Entry *def : defs.second) {
				mUses[def].erase(entry);
			}
		}

		// Remove the entry from the def-use chains
		mDefines.erase(entry);
		std::set<const IR::Entry*> &uses = mUses[entry];
		for(const IR::Entry *use : uses) {
			mDefines[use][entry->assign()].erase(entry);
		}
		mUses.erase(entry);
	}

	/*!
	 * \brief Replace the use of a symbol with another symbol
	 * \param entry Entry to examine
	 * \param oldSymbol Existing symbol
	 * \param newSymbol New symbol
	 */
	void UseDefs::replaceUse(const IR::Entry *entry, const IR::Symbol *oldSymbol, const IR::Symbol *newSymbol)
	{
		// Remove the existing def-use and use-def information
		std::set<const IR::Entry*> &oldDefs = mDefines[entry][oldSymbol];
		for(const IR::Entry *def : oldDefs) {
			mUses[def].erase(entry);
		}
		mDefines[entry].erase(oldSymbol);

		// Construct new use-def and def-use information from the underlying reaching def information
		const std::set<const IR::Entry*> &newDefs = mReachingDefs.defsForSymbol(entry, newSymbol);
		for(const IR::Entry *def : newDefs) {
			mUses[def].insert(entry);
		}
		mDefines[entry][newSymbol] = newDefs;
	}

	/*!
	 * \brief Print out use-def and def-use information
	 */
	void UseDefs::print(std::ostream &o) const
	{
		// Assign a line number to each entry
		int line = 1;
		std::map<const IR::Entry*, int> lineMap;
		for(const IR::Entry *entry : mProcedure.entries()) {
			lineMap[entry] = line;
			line++;
		}

		// Iterate through the procedure, printing out each entry along with def-use and use-def information
		for(const IR::Entry *entry : mProcedure.entries()) {
			o << lineMap[entry] << ": " << *entry;

			// Print use information
			bool printedOpen = false;
			{
				auto it = mUses.find(entry);
				if(it != mUses.end()) {
					const std::set<const IR::Entry*> &u = it->second;
					if(!u.empty()) {
						o << " [ Uses: ";
						printedOpen = true;
						for(const IR::Entry *e : u) {
							o << lineMap[e] << " ";
						}
					}
				}
			}

			// Print def information
			{
				auto it = mDefines.find(entry);
				if(it != mDefines.end()) {
					const std::map<const IR::Symbol*, std::set<const IR::Entry*>> &defs = it->second;
					for(auto &def : defs) {
						if(printedOpen) {
							o << "| ";
						} else {
							o << " [ ";
							printedOpen = true;
						}
						o << "Defs (" << def.first->name << "): ";
						for(const IR::Entry *e : def.second) {
							o << lineMap[e] << " ";
						}
					}
				}
			}

			if(printedOpen) {
				o << "]";
			}
			o << std::endl;
		}
	}
}