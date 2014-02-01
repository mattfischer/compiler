#ifndef IR_SYMBOL_H
#define IR_SYMBOL_H

#include "Front/Type.h"
#include "Front/Symbol.h"

#include <string>
#include <set>
#include <list>

namespace IR {
	/*!
	 * \brief A named symbol in the Intermediate Representation
	 */
	class Symbol {
	public:
		std::string name; //!< Symbol name
		Front::Type *type; //!< Symbol type
		Front::Symbol *symbol; //!< Front-end symbol that this one corresponds to

		/*!
		 * \brief Constructor
		 * \param _name Symbol name
		 * \param _type Symbol type
		 */
		Symbol(const std::string &_name, Front::Type *_type, Front::Symbol *_symbol) : name(_name), type(_type), symbol(_symbol) {}
	};

	typedef std::set<Symbol*> SymbolSet; //!< Convenience typedef
	typedef std::list<Symbol*> SymbolList; //!< Convenience typedef
}
#endif