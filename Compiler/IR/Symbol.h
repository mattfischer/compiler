#ifndef IR_SYMBOL_H
#define IR_SYMBOL_H

#include "Front/Type.h"

#include <string>

namespace IR {
	class Symbol {
	public:
		std::string name;
		Front::Type *type;

		Symbol(const std::string &_name, Front::Type *_type) : name(_name), type(_type) {}
	};
}
#endif