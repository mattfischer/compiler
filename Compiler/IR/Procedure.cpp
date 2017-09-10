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

	/*!
	 * \brief Print procedure
	 * \param prefix Prefix to put before each line of output
	 */
	void Procedure::print(std::ostream &o, const std::string &prefix) const
	{
		for(Entry *entry : const_cast<EntryList&>(mEntries)) {
			o << prefix << *entry << std::endl;
		}
		o << prefix << std::endl;
	}

	/*!
	 * \brief Allocate a new temporary symbol
	 * \param type Type to assign to symbol
	 * \return New symbol
	 */
	Symbol *Procedure::newTemp(int size)
	{
		std::stringstream ss;
		ss << mNextTemp++;
		std::string name = "temp" + ss.str();

		std::unique_ptr<Symbol> symbol = std::make_unique<Symbol>(name, size, nullptr);
		Symbol *ret = symbol.get();
		addSymbol(std::move(symbol));

		return ret;
	}

	/*!
	 * \brief Add an existing symbol to the procedure symbol table
	 * \param symbol Symbol to add
	 */
	void Procedure::addSymbol(std::unique_ptr<Symbol> symbol)
	{
		mSymbols.push_back(std::move(symbol));
	}

	/*!
	 * \brief Look up a symbol by name
	 * \param name Symbol name
	 * \return Symbol if found, or 0
	 */
	Symbol *Procedure::findSymbol(Front::Symbol *symbol)
	{
		for(std::unique_ptr<Symbol> &irSymbol : mSymbols) {
			if(irSymbol->symbol == symbol) {
				return irSymbol.get();
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