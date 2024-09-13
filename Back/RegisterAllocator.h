#ifndef BACK_REGISTER_ALLOCATOR_H
#define BACK_REGISTER_ALLOCATOR_H

#include "Analysis/Analysis.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include <map>

namespace Back {

/*!
 * \brief Allocate registers to each variable in a procedure
 *
 * The register allocator takes into account variables which are simultaneously live,
 * and ensures they are placed in different registers.  If there are more live variables
 * than available registers, it picks symbols to spill to the stack until an allocation
 * can be performed.  It also takes into account the registers which are not preserved
 * across procedure calls, and ensures that variables are not placed into them if their
 * value is necessary across a procedure call.
 */
class RegisterAllocator {
public:
	std::map<const IR::Symbol*, int> allocate(IR::Procedure &procedure);

private:
	std::map<const IR::Symbol*, int> tryAllocate(IR::Procedure &procedure, bool &success, Analysis::Analysis &analysis);
};

}
#endif