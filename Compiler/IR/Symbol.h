#ifndef IR_SYMBOL_H
#define IR_SYMBOL_H

#include "Front/Type.h"

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

		/*!
		 * \brief Constructor
		 * \param _name Symbol name
		 * \param _type Symbol type
		 */
		Symbol(const std::string &_name, Front::Type *_type) : name(_name), type(_type) {}
	};

	typedef std::set<Symbol*> SymbolSet; //!< Convenience typedef
	typedef std::list<Symbol*> SymbolList; //!< Convenience typedef
}
#endif