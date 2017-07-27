#ifndef IR_PROCEDURE_H
#define IR_PROCEDURE_H

#include "IR/EntryList.h"
#include "IR/Symbol.h"
#include "IR/Entry.h"

#include <string>
#include <list>
#include <vector>
#include <iostream>

namespace IR {
	/*!
	 * \brief A procedure of IR entries
	 */
	class Procedure {
	public:
		Procedure(const std::string &name);

		void print(std::ostream &o, const std::string &prefix = "") const;
		
		EntryLabel *start() const { return mStart; } //!< Start label
		EntryLabel *end() const { return mEnd; } //!< End label
		const std::string &name() const { return mName; } //!< Procedure name
		EntryList &entries() { return mEntries; } //!< Body of procedure
		const EntryList &entries() const { return mEntries; }
		SymbolList &symbols() { return mSymbols; } //!< Symbols in procedure
		const SymbolList &symbols() const { return mSymbols; }
		Symbol *newTemp(int size);
		void addSymbol(Symbol *symbol);
		Symbol *findSymbol(Front::Symbol *symbol);
		EntryLabel *newLabel();
		void setPosition(Entry *entry);

		void emit(Entry *entry);

	private:
		std::string mName; //!< Procedure name
		SymbolList mSymbols; //!< Symbol list
		EntryLabel *mStart; //!< Start label
		EntryLabel *mEnd; //!< End label

		int mNextTemp; //!< Next temporary number
		int mNextLabel; //!< Next label number
		EntryList mEntries; //!< Entry list
		bool mReturnsValue; //!< Whether the procedure returns a value
	};
}

#endif
