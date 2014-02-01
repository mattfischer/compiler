#ifndef IR_PROCEDURE_H
#define IR_PROCEDURE_H

#include "IR/EntryList.h"
#include "IR/Symbol.h"
#include "IR/Entry.h"

#include "Front/Type.h"

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
		Procedure(const std::string &name, bool returnsValue);

		void print(const std::string &prefix = "");
		
		EntryLabel *start() const { return mStart; } //!< Start label
		EntryLabel *end() const { return mEnd; } //!< End label
		const std::string &name() const { return mName; } //!< Procedure name
		EntryList &entries() { return mEntries; } //!< Body of procedure
		bool returnsValue() { return mReturnsValue; } //!< Whether the procedure returns a value
		SymbolList &symbols() { return mSymbols; } //!< Symbols in procedure
		Symbol *newTemp(Front::Type *type);
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

	typedef std::list<Procedure*> ProcedureList;
}

#endif
