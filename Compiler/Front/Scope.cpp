#include "Front/Scope.h"

namespace Front {
	/*!
	* \brief Constructor
	* \param parent Parent scope
	* \param procedure Procedure containing scope
	*/
	Scope::Scope(Scope *parent, Symbol *procedure)
	{
		mParent = parent;
		mProcedure = procedure;
	}

	/*!
	 * \brief Add an already-created symbol to the scope
	 * \param symbol Symbol to add
	 */
	bool Scope::addSymbol(Symbol *symbol)
	{
		Symbol *localSymbol = new Symbol(*symbol);
		mSymbols.push_back(localSymbol);

		return true;
	}

	/*!
	 * \brief Search for a symbol in the scope
	 * \param name Name of symbol
	 * \return Symbol if found, or 0 if not
	 */
	Symbol *Scope::findSymbol(const std::string &name)
	{
		for(unsigned int i=0; i<mSymbols.size(); i++) {
			if(mSymbols[i]->name == name) {
				return mSymbols[i];
			}
		}

		if(mParent) {
			// Search parent scope
			return mParent->findSymbol(name);
		} else {
			return 0;
		}
	}
}