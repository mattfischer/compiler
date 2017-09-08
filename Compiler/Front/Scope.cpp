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
			mParent->addChild(std::unique_ptr<Scope>(this));
		}
	}

	/*!
	 * \brief Add an already-created symbol to the scope
	 * \param symbol Symbol to add
	 */
	bool Scope::addSymbol(std::unique_ptr<Symbol> symbol)
	{
		symbol->scope = this;
		mSymbols.push_back(std::move(symbol));
		return true;
	}

	/*!
	 * \brief Search for a symbol in the scope
	 * \param name Name of symbol
	 * \return Symbol if found, or 0 if not
	 */
	Symbol *Scope::findSymbol(const std::string &name)
	{
		for(std::unique_ptr<Symbol> &symbol : mSymbols) {
			if(symbol->name == name) {
				return symbol.get();
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
	void Scope::addChild(std::unique_ptr<Scope> child)
	{
		mChildren.push_back(std::move(child));
	}

	/*!
	 * \brief Return all symbols in scope and children
	 * \return Symbols
	 */
	std::vector<Symbol*> Scope::allSymbols()
	{
		std::vector<Symbol*> result;
		for (std::unique_ptr<Symbol> &symbol : mSymbols) {
			result.push_back(symbol.get());
		}

		for(const std::unique_ptr<Scope> &child : mChildren) {
			std::vector<Symbol*> childSymbols = child->allSymbols();
			result.insert(result.end(), childSymbols.begin(), childSymbols.end());
		}

		return result;
	}
}