#include "Back/AsmParser.h"

namespace Back {

struct NameInt {
	char *name;
	int value;
};

struct NameTwoInt {
	char *name;
	int value1;
	int value2;
};

/*!
 * \brief Constructor
 * \param tokenizer Tokenizer
 */
AsmParser::AsmParser(AsmTokenizer &tokenizer)
	: Parser(tokenizer)
{
}

/*!
 * \brief Parse a program
 * \return Assembled program, or 0 if error
 */
VM::Program *AsmParser::parse()
{
	try {
		// Parse the stream
		return parseProgram();
	} catch(Input::Parser::ParseException parseException) {
		// Collect the error information from the exception
		setError(parseException.message(), parseException.line(), parseException.column());
		return 0;
	}
}

/*!
 * \brief Parse a program
 * \return Assembled program
 */
VM::Program *AsmParser::parseProgram()
{
	VM::Program *program = new VM::Program;

	// Scan each procedure in turn
	while(matchLiteral("defproc") || matchLiteral("defdata")) {
		consume();

		std::string name = next().text;
		expect(AsmTokenizer::TypeIdentifier);

		program->symbols[name] = (int)program->instructions.size();
		parseProcedure(program);
	}

	expect(AsmTokenizer::TypeEnd);

	return program;
}

/*!
 * \brief Parse a procedure
 * \param program Program to write to
 */
void AsmParser::parseProcedure(VM::Program *program)
{
	VM::Instruction instr;
	std::map<int, std::string> labelRefs;
	std::map<std::string, int> labels;

	while(true) {
		// Break out when the procedure ends
		if(match(AsmTokenizer::TypeEnd) || matchLiteral("defproc") || matchLiteral("defdata")) {
			break;
		}

		int offset = (int)program->instructions.size();
		if(parseStdInstr(instr) || parseIndInstr(instr) || parseImmInstr(instr)
			|| parseMultInstr(instr) || parseLabelRef(instr, offset, labelRefs)
			|| parseExternalRef(instr, offset, program->relocations)) {
			// Parse a regular instruction
			program->instructions.resize(offset + 4);
			std::memcpy(&program->instructions[offset], &instr, 4);
		} else if(matchLiteral("string")) {
			// Parse a string constant
			consume();
			std::string value = next().text;
			expect(AsmTokenizer::TypeString);
			int newSize = offset + (int)value.size() + 1;
			if(newSize % 4 > 0) {
				newSize += 4 - (newSize % 4);
			}
			program->instructions.resize(newSize);
			std::memcpy(&program->instructions[offset], value.c_str(), value.size());
			program->instructions[offset + value.size()] = '\0';
		} else if(matchLiteral("addr")) {
			consume();
			std::string name = next().text;
			expect(AsmTokenizer::TypeIdentifier);
			program->instructions.resize(offset + 4);
			int value = 0;
			std::memcpy(&program->instructions[offset], &value, sizeof(value));
			VM::Program::Relocation relocation;
			relocation.offset = offset;
			relocation.type = VM::Program::Relocation::Type::Absolute;
			relocation.symbol = name;
			program->relocations.push_back(relocation);
		} else {
			// Parse a literal
			std::string text = next().text;
			consume();
			expectLiteral(":");
			labels[text] = offset;
		}
	}

	// Patch up label references
	for(std::map<int, std::string>::iterator it = labelRefs.begin(); it != labelRefs.end(); it++) {
		int offset = it->first;
		const std::string &target = it->second;

		VM::Instruction instr;
		std::memcpy(&instr, &program->instructions[offset], 4);
		if(instr.type == VM::InstrTwoAddr && instr.two.type == VM::TwoAddrAddImm) {
			int targetOffset = labels[target];
			instr.two.imm = targetOffset - offset;
		} else if(instr.type == VM::InstrThreeAddr && (instr.three.type == VM::ThreeAddrAddCond || instr.three.type == VM::ThreeAddrAddNCond)) {
			int targetOffset = labels[target];
			instr.three.imm = targetOffset - offset;
		}

		std::memcpy(&program->instructions[offset], &instr, 4);
	}
}

/*!
 * \brief Parse a standard-format instruction
 * \param instr Instruction to write to
 */
bool AsmParser::parseStdInstr(VM::Instruction &instr)
{
	static const NameTwoInt stdOps[] = {
		{ "sub", VM::InstrThreeAddr, VM::ThreeAddrSub },
		{ "equ", VM::InstrThreeAddr, VM::ThreeAddrEqual },
		{ "neq", VM::InstrThreeAddr, VM::ThreeAddrNEqual },
		{ "lt", VM::InstrThreeAddr, VM::ThreeAddrLessThan },
		{ "lte", VM::InstrThreeAddr, VM::ThreeAddrLessThanE },
		{ "gt", VM::InstrThreeAddr, VM::ThreeAddrGreaterThan },
		{ "gte", VM::InstrThreeAddr, VM::ThreeAddrGreaterThanE },
		{ "or", VM::InstrThreeAddr, VM::ThreeAddrOr },
		{ "and", VM::InstrThreeAddr, VM::ThreeAddrAnd },
		{ "new", VM::InstrTwoAddr, VM::TwoAddrNew },
		{ "calli", VM::InstrOneAddr, VM::OneAddrCall }
	};

	for(const NameTwoInt &op : stdOps) {
		if(next().text == op.name) {
			switch(op.value1) {
				case VM::InstrThreeAddr:
				{
					consume();
					int lhs = parseReg();
					expectLiteral(",");
					int rhs1 = parseReg();
					expectLiteral(",");
					int rhs2 = parseReg();

					instr = VM::Instruction::makeThreeAddr(op.value2, lhs, rhs1, rhs2, 0);
					return true;
				}
				case VM::InstrTwoAddr:
				{
					consume();
					int lhs = parseReg();
					expectLiteral(",");
					int rhs = parseReg();

					instr = VM::Instruction::makeTwoAddr(op.value2, lhs, rhs, 0);
					return true;
				}
				case VM::InstrOneAddr: 
				{
					consume();
					int lhs = parseReg();

					instr = VM::Instruction::makeOneAddr(op.value2, lhs, 0);
					return true;
				}
			}
		}
	}
	return false;
}

/*!
 * \brief Parse an indexed instruction
 * \param instr Instruction to write to
 */
bool AsmParser::parseIndInstr(VM::Instruction &instr)
{
	static const NameTwoInt indOps[] = {
		{ "ldr", VM::TwoAddrLoad, VM::ThreeAddrLoad },
		{ "str", VM::TwoAddrStore, VM::ThreeAddrStore },
		{ "ldb", VM::TwoAddrLoadByte, VM::ThreeAddrLoadByte },
		{ "stb", VM::TwoAddrStoreByte, VM::ThreeAddrStoreByte },
	};

	for(const NameTwoInt &op : indOps) {
		if(next().text == op.name) {
			consume();
			int lhs = parseReg();
			expectLiteral(",");
			expectLiteral("[");
			int rhs1 = parseReg();
			if(matchLiteral("]")) {
				consume();
				instr = VM::Instruction::makeTwoAddr(op.value1, lhs, rhs1, 0);
			} else {
				expectLiteral(",");
				if(matchLiteral("#")) {
					consume();
					int imm = std::atoi(next().text.c_str());
					expect(AsmTokenizer::TypeNumber);
					expectLiteral("]");
					instr = VM::Instruction::makeTwoAddr(op.value1, lhs, rhs1, imm);
				} else {
					int rhs2 = parseReg();
					int imm = 0;
					if(matchLiteral(",")) {
						consume();
						expectLiteral("#");
						imm = std::atoi(next().text.c_str());
						expect(AsmTokenizer::TypeNumber);
					}
					expectLiteral("]");
					instr = VM::Instruction::makeThreeAddr(op.value2, lhs, rhs1, rhs2, imm);
				}
			}

			return true;
		}
	}

	return false;
}

/*!
 * \brief Parse an immediate-format instruction
 * \param instr Instruction to write to
 */
bool AsmParser::parseImmInstr(VM::Instruction &instr)
{
	static const NameTwoInt imm23Ops[] = {
		{ "add", VM::TwoAddrAddImm, VM::ThreeAddrAdd },
		{ "mult", VM::TwoAddrMultImm, VM::ThreeAddrMult },
		{ "div", VM::TwoAddrDivImm, VM::ThreeAddrDiv },
		{ "mod", VM::TwoAddrModImm, VM::ThreeAddrMod },
	};

	for(const NameTwoInt &op : imm23Ops) {
		if(next().text == op.name) {
			consume();
			int lhs = parseReg();
			expectLiteral(",");
			int rhs1 = parseReg();
			expectLiteral(",");

			if(matchLiteral("#")) {
				consume();
				int imm = std::atoi(next().text.c_str());
				expect(AsmTokenizer::TypeNumber);
				instr = VM::Instruction::makeTwoAddr(op.value1, lhs, rhs1, imm);
			} else {
				int rhs2 = parseReg();
				instr = VM::Instruction::makeThreeAddr(op.value2, lhs, rhs1, rhs2, 0);
			}
			return true;
		}
	}

	static const NameTwoInt imm12Ops[] = {
		{ "mov", VM::OneAddrLoadImm, VM::TwoAddrAddImm }
	};

	for(const NameTwoInt &op : imm12Ops) {
		if(next().text == op.name) {
			consume();
			int lhs = parseReg();
			expectLiteral(",");

			if(matchLiteral("#")) {
				consume();
				int imm = std::atoi(next().text.c_str());
				expect(AsmTokenizer::TypeNumber);
				instr = VM::Instruction::makeOneAddr(op.value1, lhs, imm);
			} else {
				int rhs = parseReg();
				instr = VM::Instruction::makeTwoAddr(op.value2, lhs, rhs, 0);
			}
			return true;
		}
	}

	return false;
}

/*!
 * \brief Parse a multi-register instruction
 * \param instr Instruction to write to
 */
bool AsmParser::parseMultInstr(VM::Instruction &instr)
{
	static const NameTwoInt multOps[] = {
		{ "stm", VM::MultRegStore, 0 },
		{ "ldm", VM::MultRegLoad, 0 },
	};

	for(const NameTwoInt &op : multOps) {
		if(next().text == op.name) {
			consume();
			int lhs = parseReg();
			expectLiteral(",");
			expectLiteral("{");
			int regs = 0;
			while(true) {
				int regStart = parseReg();
				int regEnd;
				if(matchLiteral("-")) {
					consume();
					regEnd = parseReg();
				} else {
					regEnd = regStart;
				}

				for(int j=regStart; j<=regEnd; j++) {
					regs |= (1 << j);
				}

				if(matchLiteral(",")) {
					consume();
				} else {
					break;
				}
			}
			expectLiteral("}");

			instr = VM::Instruction::makeMultReg(op.value1, lhs, regs);
			return true;
		}
	}

	return false;
}

/*!
 * \brief Parse a jump instruction
 * \param instr Instruction to write to
 * \param offset Offset of current instruction in procedure
 * \param labelRefs Label reference map
 */
bool AsmParser::parseLabelRef(VM::Instruction &instr, int offset, std::map<int, std::string> &labelRefs)
{
	if(next().text == "jmp") {
		consume();
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegPC, 0);
		labelRefs[offset] = target;
		return true;
	}

	if(next().text == "cjmp") {
		consume();
		int pred = parseReg();
		expectLiteral(",");
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddCond, VM::RegPC, pred, VM::RegPC, 0);
		labelRefs[offset] = target;
		return true;
	}

	if(next().text == "ncjmp") {
		consume();
		int pred = parseReg();
		expectLiteral(",");
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddNCond, VM::RegPC, pred, VM::RegPC, 0);
		labelRefs[offset] = target;
		return true;
	}

	return false;
}

/*!
 * \brief Parse a reference to an external symbol
 * \param instr Instruction to write to
 * \param offset Offset of current instruction in procedure
 * \param relocations Relocation map
 */
bool AsmParser::parseExternalRef(VM::Instruction &instr, int offset, std::vector<VM::Program::Relocation> &relocations)
{
	if(next().text == "call") {
		consume();
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeOneAddr(VM::OneAddrCall, VM::RegPC, 0);
		VM::Program::Relocation relocation;
		relocation.offset = offset;
		relocation.type = VM::Program::Relocation::Type::Call;
		relocation.symbol = target;
		relocations.push_back(relocation);
		return true;
	}

	if(matchLiteral("lea")) {
		consume();
		int lhs = parseReg();
		expectLiteral(",");
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, lhs, VM::RegPC, 0);
		VM::Program::Relocation relocation;
		relocation.offset = offset;
		relocation.type = VM::Program::Relocation::Type::AddPCRel;
		relocation.symbol = target;
		relocations.push_back(relocation);
		return true;
	}

	return false;
}

/*!
 * \brief Parse a register name
 * \return Register number
 */
int AsmParser::parseReg()
{
	NameInt regs[] = { { "sp", 13 }, { "lr", 14 }, { "pc", 15 } };

	const std::string &text = next().text;

	if(text[0] == 'r') {
		std::string num = text.substr(1);
		consume();
		return std::atoi(num.c_str());
	}

	for(const NameInt &reg : regs) {
		if(reg.name == text) {
			consume();
			return reg.value;
		}
	}

	errorExpected("register");
	return -1;
}

}