#include "Front/Parser.h"
#include "Middle/Optimizer.h"
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

	Middle::Optimizer::optimize(program);

	printf("IR:\n");
	program->print();
	printf("\n");

	std::vector<VM::Instruction> instrs = Back::CodeGenerator::generate(program->main());
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