#include "Front/Tokenizer.h"
#include "Front/Parser.h"
#include "Front/TypeChecker.h"
#include "Front/IRGenerator.h"

#include "Middle/Optimizer.h"
#include "Middle/ErrorCheck.h"
#include "Back/CodeGenerator.h"

#include "VM/Instruction.h"
#include "VM/Interp.h"

#include <stdio.h>

int main(int arg, char *argv[])
{
	Front::Tokenizer tokenizer("input.lang");
	Front::Parser parser(tokenizer);

	Front::Node *syntaxTree = parser.parse();
	Front::TypeChecker typeChecker;
	bool result = typeChecker.check(syntaxTree);

	IR::Program *irProgram = 0;
	if(result) {
		Front::IRGenerator generator;
		irProgram = generator.generate(syntaxTree);
	}

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