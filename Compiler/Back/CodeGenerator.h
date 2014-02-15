#ifndef BACK_CODE_GENERATOR_H
#define BACK_CODE_GENERATOR_H

#include "VM/Program.h"

#include "IR/Procedure.h"
#include "IR/Program.h"

#include <vector>
#include <map>

/*!
 * \brief Back-end functions for the compiler
 *
 * The Back namespace contains the register allocater and code generator for the compiler.
 */
namespace Back {
	/*!
	 * \brief Convert the IR procedure into final code instructions
	 */
	class CodeGenerator {
	public:
		static VM::Program generate(IR::Program *irProgram);

	private:
		static void generateProcedure(IR::Procedure *procedure, std::vector<unsigned char> &data, const std::map<IR::Procedure*, int> &procedureMap);
	};
}
#endif