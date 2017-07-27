#ifndef BACK_CODE_GENERATOR_H
#define BACK_CODE_GENERATOR_H

#include "IR/Procedure.h"
#include "IR/Program.h"

#include <iostream>

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
		static void generate(IR::Program *irProgram, std::ostream &stream);

	private:
		static void generateProcedure(IR::Procedure &procedure, std::ostream &stream);
		static void generateData(IR::Data &data, std::ostream &stream);
	};
}
#endif