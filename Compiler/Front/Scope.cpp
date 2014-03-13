#include "Front/Scope.h"

namespace Front {
	/*!
	* \brief Constructor
	* \param parent Parent scope
	* \param procedure Procedure containing scope
	*/
	Scope::Scope(Scope *parent, TypeStruct *classType)
	{
		mParent = parent;
		mClassType = classType;

		if(mParent) {
			mParent->addChild(this);
		}
	}

	/*!
	 * \brief Add an already-created symbol to the scope
	 * \param symbol Symbol to add
	 */
	bool Scope::addSymbol(Symbol *symbol)
	{
		symbol->scope = this;
		mSymbols.push_back(symbol);
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

	/*!
	 * \brief Add child scope
	 * \param child Child scope
	 */
	void Scope::addChild(Scope *child)
	{
		mChildren.push_back(child);
	}

	/*!
	 * \brief Return all symbols in scope and children
	 * \return Symbols
	 */
	std::vector<Symbol*> Scope::symbols()
	{
		std::vector<Symbol*> result = mSymbols;
		for(unsigned int i=0; i<mChildren.size(); i++) {
			std::vector<Symbol*> childSymbols = mChildren[i]->symbols();
			result.insert(result.end(), childSymbols.begin(), childSymbols.end());
		}

		return result;
	}
}