#ifndef FRONT_SCOPE_H
#define FRONT_SCOPE_H

#include "Front/Symbol.h"

#include <vector>
#include <memory>

namespace Front {
	/*!
	 * \brief A collection of variables that are in scope at some point in the program
	 */
	class Scope {
	public:
		Scope(Scope *parent = 0);
		Scope(Scope *parent, std::shared_ptr<TypeStruct> &classType);

		Scope *parent() { return mParent; }
		bool addSymbol(std::unique_ptr<Symbol> symbol);
		Symbol *findSymbol(const std::string &name);
		std::vector<Symbol*> allSymbols();
		std::vector<std::unique_ptr<Symbol>> &symbols() { return mSymbols; }

		void addChild(std::unique_ptr<Scope> child);
		std::shared_ptr<TypeStruct> classType() { return mClassType; }

	private:
		Scope *mParent; //!< Parent scope
		std::vector<std::unique_ptr<Scope>> mChildren; //!< Child scopes
		std::vector<std::unique_ptr<Symbol>> mSymbols; //!< Collection of symbols
		std::shared_ptr<TypeStruct> mClassType;
	};
}

#endif
