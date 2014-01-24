#ifndef FRONT_SYMBOL_H
#define FRONT_SYMBOL_H

#include "Front/Type.h"

#include <string>

namespace Front {

	/*!
	 * \brief A symbol in the program
	 */
	struct Symbol {
		Type *type; //!< Type of variable
		std::string name; //!< Variable name

		/*!
		 * \brief Constructor
		 * \param _type Symbol type
		 * \param _name Symbol name
		 */
		Symbol(Type *_type, const std::string &_name) : type(_type), name(_name) {}

		/*!
		 * \brief Copy constructor
		 * \param other Copy source
		 */
		Symbol(const Symbol &other) : type(other.type), name(other.name) {}
	};
}

#endif