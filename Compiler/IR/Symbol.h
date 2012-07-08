#ifndef IR_SYMBOL_H
#define IR_SYMBOL_H

#include <string>
#include <vector>

struct Type;

namespace IR {
	struct Entry;

	struct Symbol {
		std::string name;
		Type *type;
		int uses;
		std::vector<Entry*> assigns;

		int *value;
		Symbol(const std::string &_name, Type *_type) : name(_name), type(_type), uses(0), value(0) {}

		void addAssign(Entry *entry);
		void removeAssign(Entry *entry);
	};
}
#endif