#include "IR/Procedure.h"

#include "IR/Symbol.h"
#include "IR/Entry.h"

#include "Front/Type.h"

#include <sstream>

namespace IR {
	Procedure::Procedure(const std::string &name)
	{
		mNextTemp = 0;
		mNextLabel = 1;
		mName = name;
		mStart = new EntryLabel("start");
		mEnd = new EntryLabel("end");
		mEntries.push_back(mStart);
		mEntries.push_back(mEnd);
	}

	void Procedure::print(const std::string &prefix)
	{
		std::cout << prefix << "Symbols:" << std::endl;
		for(SymbolList::const_iterator it = mSymbols.begin(); it != mSymbols.end(); it++) {
			Symbol *symbol = *it;
			std::cout << prefix << "  " << symbol->type->name << " " << symbol->name << std::endl;
		}
		std::cout << prefix << std::endl;
		std::cout << prefix << "Body:" << std::endl;
		for(IR::EntryList::iterator itEntry = mEntries.begin(); itEntry != mEntries.end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			std::cout << prefix << *entry << std::endl;
		}
		std::cout << prefix << std::endl;
	}

	Symbol *Procedure::newTemp(Front::Type *type)
	{
		std::stringstream ss;
		ss << mNextTemp++;
		std::string name = "temp" + ss.str();

		return addSymbol(name, type);
	}

	Symbol *Procedure::addSymbol(const std::string &name, Front::Type *type)
	{
		Symbol *symbol = new Symbol(name, type);
		addSymbol(symbol);

		return symbol;
	}

	void Procedure::addSymbol(Symbol *symbol)
	{
		mSymbols.push_back(symbol);
	}

	Symbol *Procedure::findSymbol(const std::string &name)
	{
		for(SymbolList::iterator it = mSymbols.begin(); it != mSymbols.end(); it++) {
			Symbol *symbol = *it;
			if(symbol->name == name) {
				return symbol;
			}
		}

		return 0;
	}

	EntryLabel *Procedure::newLabel()
	{
		std::stringstream ss;
		ss << mNextLabel++;
		std::string name = "bb" + ss.str();
		return new EntryLabel(name);
	}

	void Procedure::emit(Entry *entry)
	{
		mEntries.insert(mEnd, entry);
	}
}