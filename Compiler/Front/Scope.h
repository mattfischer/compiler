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
		Scope(Scope *parent = 0, TypeStruct *classType = 0);

		Scope *parent() { return mParent; }
		bool addSymbol(Symbol *symbol);
		Symbol *findSymbol(const std::string &name);
		std::vector<Symbol*> symbols();
		void addChild(Scope *child);
		TypeStruct *classType() { return mClassType; }

	private:
		Scope *mParent; //!< Parent scope
		std::vector<Scope*> mChildren; //!< Child scopes
		std::vector<Symbol*> mSymbols; //!< Collection of symbols
		TypeStruct *mClassType;
	};
}

#endif
