#include "IR/Procedure.h"

#include "IR/Symbol.h"
#include "IR/Entry.h"

#include "Front/Type.h"

#include <sstream>

namespace IR {
	/*!
	 * \brief Constructor
	 * \param name Procedure name
	 */
	Procedure::Procedure(const std::string &name, bool returnsValue)
	{
		mNextTemp = 0;
		mNextLabel = 1;
		mName = name;
		mStart = new EntryLabel("start");
		mEnd = new EntryLabel("end");
		mEntries.push_back(mStart);
		mEntries.push_back(mEnd);
		mReturnsValue = returnsValue;
	}

	/*!
	 * \brief Print procedure
	 * \param prefix Prefix to put before each line of output
	 */
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

	/*!
	 * \brief Allocate a new temporary symbol
	 * \param type Type to assign to symbol
	 * \return New symbol
	 */
	Symbol *Procedure::newTemp(Front::Type *type)
	{
		std::stringstream ss;
		ss << mNextTemp++;
		std::string name = "temp" + ss.str();

		IR::Symbol *symbol = new Symbol(name, type, 0);
		addSymbol(symbol);

		return symbol;
	}

	/*!
	 * \brief Add an existing symbol to the procedure symbol table
	 * \param symbol Symbol to add
	 */
	void Procedure::addSymbol(Symbol *symbol)
	{
		mSymbols.push_back(symbol);
	}

	/*!
	 * \brief Look up a symbol by name
	 * \param name Symbol name
	 * \return Symbol if found, or 0
	 */
	Symbol *Procedure::findSymbol(Front::Symbol *symbol)
	{
		for(SymbolList::iterator it = mSymbols.begin(); it != mSymbols.end(); it++) {
			Symbol *irSymbol = *it;
			if(irSymbol->symbol == symbol) {
				return irSymbol;
			}
		}

		return 0;
	}

	/*!
	 * \brief Allocate a new label
	 * \return New label
	 */
	EntryLabel *Procedure::newLabel()
	{
		std::stringstream ss;
		ss << mNextLabel++;
		std::string name = "bb" + ss.str();
		return new EntryLabel(name);
	}

	/*!
	 * \brief Add an entry to the end of the procedure
	 * \param entry Entry to add
	 */
	void Procedure::emit(Entry *entry)
	{
		mEntries.insert(mEnd, entry);
	}
}