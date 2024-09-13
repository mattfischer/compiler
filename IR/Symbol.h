#ifndef IR_SYMBOL_H
#define IR_SYMBOL_H

#include <string>
#include <set>
#include <list>

namespace Front {
	struct Symbol;
}

namespace IR {
	/*!
	 * \brief A named symbol in the Intermediate Representation
	 */
	class Symbol {
	public:
		std::string name; //!< Symbol name
		int size; //!< Symbol data size
		const Front::Symbol *symbol; //!< Front-end symbol that this one corresponds to

		/*!
		 * \brief Constructor
		 * \param _name Symbol name
		 * \param _symbol Front-end symbol
		 */
		Symbol(const std::string &_name, int _size, const Front::Symbol *_symbol) : name(_name), size(_size), symbol(_symbol) {}
	};
}
#endif