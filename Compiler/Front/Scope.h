#ifndef FRONT_SCOPE_H
#define FRONT_SCOPE_H

#include "Front/Symbol.h"

#include <vector>

namespace Front {
	/*!
	 * \brief A collection of variables that are in scope at some point in the program
	 */
	class Scope {
	public:
		Scope(Scope *parent = 0);

		bool addSymbol(Symbol *symbol);
		Symbol *findSymbol(const std::string &name);
		std::vector<Symbol*> symbols();
		void addChild(Scope *child);

	private:
		Scope *mParent; //!< Parent scope
		std::vector<Scope*> mChildren; //!< Child scopes
		std::vector<Symbol*> mSymbols; //!< Collection of symbols
	};
}

#endif
