#ifndef BACK_ASM_PARSER_H
#define BACK_ASM_PARSER_H

#include "Input/Parser.h"

#include "Back/AsmTokenizer.h"
#include "VM/Program.h"

#include <string>
#include <map>

namespace Back {

/*!
 * \brief Parser for the assembler
 *
 * Consumes a token list from the tokenizer and converts it to a compiled program
 */
class AsmParser : public Input::Parser {
public:
	AsmParser(AsmTokenizer &tokenizer);

	VM::Program *parse();

private:
	VM::Program *parseProgram();
	void parseProcedure(VM::Program *program);

	bool parseStdInstr(VM::Instruction &instr);
	bool parseIndInstr(VM::Instruction &instr);
	bool parseImmInstr(VM::Instruction &instr);
	bool parseMultInstr(VM::Instruction &instr);
	bool parseLabelRef(VM::Instruction &instr, int offset, std::map<int, std::string> &labelRefs);
	bool parseCall(VM::Instruction &instr, int offset, std::vector<VM::Program::Relocation> &relocations);

	int parseReg();
};
}
#endif