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
		const std::vector<Symbol*> &symbols() { return mSymbols; } //!< Symbols

	private:
		Scope *mParent; //!< Parent scope
		std::vector<Symbol*> mSymbols; //!< Collection of symbols
	};
}

#endif
