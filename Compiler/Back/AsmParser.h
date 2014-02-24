#ifndef BACK_ASM_PARSER_H
#define BACK_ASM_PARSER_H

#include "Input/Parser.h"

#include "Back/AsmTokenizer.h"
#include "VM/Program.h"

#include <string>
#include <map>

namespace Back {

class AsmParser : public Input::Parser {
public:
	AsmParser(AsmTokenizer &tokenizer);

	VM::Program *parse();

private:
	VM::Program *parseProgram();
	void parseProcedure(VM::Program *program, std::map<std::string, int> &procedureMap);

	bool parseStdInstr(VM::Instruction &instr);
	bool parseIndInstr(VM::Instruction &instr);
	bool parseImmInstr(VM::Instruction &instr);
	bool parseMultInstr(VM::Instruction &instr);
	bool parseJumpCall(VM::Instruction &instr, int offset, std::map<int, std::string> &jumps);
	bool parseStringLoad(VM::Instruction &instr, int offset, std::map<int, std::string> &stringLoads);

	int parseReg();
};
}
#endif