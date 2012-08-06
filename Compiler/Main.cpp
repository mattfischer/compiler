#include "Front/Parser.h"
#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"
#include "Back/CodeGenerator.h"

#include "VM/Instruction.h"
#include "VM/Interp.h"

#include <stdio.h>

int main(int arg, char *argv[])
{
	IR::Program *program = Front::Parser::parse("input.lang");
	if(!program) {
		return 1;
	}

	if(!Middle::ErrorCheck::check(program)) {
		return 1;
	}

	Middle::Optimizer::optimize(program);

	std::vector<VM::Instruction> instrs = Back::CodeGenerator::generate(program);
	printf("Code:\n");
	for(unsigned int i = 0; i < instrs.size(); i++) {
		printf("  ");
		instrs[i].print();
		printf("\n");
	}
	printf("\n");

	printf("Output:\n");
	VM::Interp::run(instrs);

	return 0;
}