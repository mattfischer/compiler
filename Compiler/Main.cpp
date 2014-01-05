#include "Front/Parser.h"

#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"
#include "Back/CodeGenerator.h"

#include "VM/Instruction.h"
#include "VM/Interp.h"

#include <stdio.h>

int main(int arg, char *argv[])
{
	IR::Program *irProgram = Front::Parser::parse("input.lang");
	if(!irProgram) {
		return 1;
	}

	if(!Middle::ErrorCheck::check(irProgram)) {
		return 1;
	}

	Middle::Optimizer::optimize(irProgram);

	VM::Program vmProgram = Back::CodeGenerator::generate(irProgram);
	printf("Code:\n");
	for(unsigned int i = 0; i < vmProgram.instructions.size(); i++) {
		printf("  ");
		vmProgram.instructions[i].print();
		printf("\n");
	}
	printf("\n");

	printf("Output:\n");
	VM::Interp::run(vmProgram);

	return 0;
}