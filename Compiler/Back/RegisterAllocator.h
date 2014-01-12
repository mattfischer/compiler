#ifndef BACK_REGISTER_ALLOCATOR_H
#define BACK_REGISTER_ALLOCATOR_H

#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include <map>

namespace Back {

class RegisterAllocator {
public:
	std::map<IR::Symbol*, int> allocate(IR::Procedure *procedure);

private:
	std::map<IR::Symbol*, int> tryAllocate(IR::Procedure *procedure, bool &success);
};

}
#endif