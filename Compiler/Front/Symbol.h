#ifndef FRONT_SYMBOL_H
#define FRONT_SYMBOL_H

#include "Front/Type.h"

#include <string>

namespace Front {

	class Scope;

	/*!
	 * \brief A symbol in the program
	 */
	struct Symbol {
		std::shared_ptr<Type> type; //!< Type of variable
		std::string name; //!< Variable name
		Scope *scope; //!< Containing scope

		/*!
		 * \brief Constructor
		 * \param _type Symbol type
		 * \param _name Symbol name
		 */
		Symbol(std::shared_ptr<Type> _type, const std::string &_name) : type(_type), name(_name), scope(0) {}

		/*!
		 * \brief Copy constructor
		 * \param other Copy source
		 */
		Symbol(const Symbol &other) : type(other.type), name(other.name), scope(other.scope) {}
	};
}

#endif