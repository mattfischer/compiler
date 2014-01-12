#ifndef IR_PROCEDURE_H
#define IR_PROCEDURE_H

#include "IR/EntryList.h"

#include "Front/Type.h"

#include <string>
#include <list>
#include <vector>
#include <iostream>

namespace IR {
	class Symbol;
	class Entry;
	struct EntryLabel;

	class Procedure {
	public:
		Procedure(const std::string &name);

		void print(const std::string &prefix = "");
		
		EntryLabel *start() const { return mStart; }
		EntryLabel *end() const { return mEnd; }
		const std::string &name() const { return mName; }
		EntryList &entries() { return mEntries; }

		typedef std::list<Symbol*> SymbolList;
		SymbolList &symbols() { return mSymbols; }
		Symbol *newTemp(Front::Type *type);
		Symbol *addSymbol(const std::string &name, Front::Type *type);
		void addSymbol(Symbol *symbol);
		Symbol *findSymbol(const std::string &name);
		EntryLabel *newLabel();
		void setPosition(Entry *entry);

		void emit(Entry *entry);

	private:
		std::string mName;
		SymbolList mSymbols;
		EntryLabel *mStart;
		EntryLabel *mEnd;

		int mNextTemp;
		int mNextLabel;
		EntryList mEntries;
	};
}

#endif
