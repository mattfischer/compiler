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

#define N_ENTRIES(arr) (sizeof(arr) / sizeof(arr[0]))

AsmParser::AsmParser(AsmTokenizer &tokenizer)
	: Parser(tokenizer)
{
}

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

VM::Program *AsmParser::parseProgram()
{
	VM::Program *program = new VM::Program;

	while(matchLiteral("defproc")) {
		consume();

		std::string name = next().text;
		expect(AsmTokenizer::TypeIdentifier);

		program->symbols[name] = (int)program->instructions.size();
		parseProcedure(program);
	}

	expect(AsmTokenizer::TypeEnd);

	return program;
}

void AsmParser::parseProcedure(VM::Program *program)
{
	VM::Instruction instr;
	std::map<int, std::string> jumps;
	std::map<std::string, int> labels;
	std::map<std::string, int> strings;
	std::map<int, std::string> stringLoads;

	while(true) {
		if(match(AsmTokenizer::TypeEnd) || matchLiteral("defproc")) {
			break;
		}

		int offset = (int)program->instructions.size();
		if(parseStdInstr(instr) || parseIndInstr(instr) || parseImmInstr(instr)
			|| parseMultInstr(instr) || parseStringLoad(instr, offset, stringLoads)
			|| parseJumpCall(instr, offset, jumps)) {
			program->instructions.resize(offset + 4);
			std::memcpy(&program->instructions[offset], &instr, 4);
		} else if(matchLiteral("string")) {
			consume();
			std::string name = next().text;
			expect(AsmTokenizer::TypeIdentifier);
			expectLiteral(",");
			std::string value = next().text;
			expect(AsmTokenizer::TypeString);
			int newSize = offset + value.size() + 1;
			if(newSize % 4 > 0) {
				newSize += 4 - (newSize % 4);
			}
			program->instructions.resize(newSize);
			std::memcpy(&program->instructions[offset], value.c_str(), value.size());
			program->instructions[offset + value.size()] = '\0';
			strings[name] = offset;
		} else {
			std::string text = next().text;
			consume();
			expectLiteral(":");
			labels[text] = offset;
		}
	}

	for(std::map<int, std::string>::iterator it = jumps.begin(); it != jumps.end(); it++) {
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
		} else if(instr.type == VM::InstrOneAddr && instr.one.type == VM::OneAddrCall) {
			program->imports[offset] = target;
		}

		std::memcpy(&program->instructions[offset], &instr, 4);
	}

	for(std::map<int, std::string>::iterator it = stringLoads.begin(); it != stringLoads.end(); it++) {
		int offset = it->first;
		const std::string &name = it->second;
		VM::Instruction instr;
		std::memcpy(&instr, &program->instructions[offset], 4);
		int dataOffset = strings[name];
		instr.two.imm = dataOffset - offset;
		std::memcpy(&program->instructions[offset], &instr, 4);
	}
}

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
		{ "concat", VM::InstrThreeAddr, VM::ThreeAddrConcat },
		{ "new", VM::InstrTwoAddr, VM::TwoAddrNew },
		{ "strint", VM::InstrTwoAddr, VM::TwoAddrStringInt },
		{ "strbool", VM::InstrTwoAddr, VM::TwoAddrStringBool },
		{ "print", VM::InstrOneAddr, VM::OneAddrPrint }
	};

	for(int i=0; i<N_ENTRIES(stdOps); i++) {
		if(next().text == stdOps[i].name) {
			switch(stdOps[i].value1) {
				case VM::InstrThreeAddr:
				{
					consume();
					int lhs = parseReg();
					expectLiteral(",");
					int rhs1 = parseReg();
					expectLiteral(",");
					int rhs2 = parseReg();

					instr = VM::Instruction::makeThreeAddr(stdOps[i].value2, lhs, rhs1, rhs2, 0);
					return true;
				}
				case VM::InstrTwoAddr:
				{
					consume();
					int lhs = parseReg();
					expectLiteral(",");
					int rhs = parseReg();

					instr = VM::Instruction::makeTwoAddr(stdOps[i].value2, lhs, rhs, 0);
					return true;
				}
				case VM::InstrOneAddr: 
				{
					consume();
					int lhs = parseReg();

					instr = VM::Instruction::makeOneAddr(stdOps[i].value2, lhs, 0);
					return true;
				}
			}
		}
	}
	return false;
}

bool AsmParser::parseIndInstr(VM::Instruction &instr)
{
	static const NameTwoInt indOps[] = {
		{ "ldr", VM::TwoAddrLoad, VM::ThreeAddrLoad },
		{ "str", VM::TwoAddrStore, VM::ThreeAddrStore },
		{ "ldb", VM::TwoAddrLoadByte, VM::ThreeAddrLoadByte },
		{ "stb", VM::TwoAddrStoreByte, VM::ThreeAddrStoreByte },
	};

	for(int i=0; i<N_ENTRIES(indOps); i++) {
		if(next().text == indOps[i].name) {
			consume();
			int lhs = parseReg();
			expectLiteral(",");
			expectLiteral("[");
			int rhs1 = parseReg();
			if(matchLiteral("]")) {
				consume();
				instr = VM::Instruction::makeTwoAddr(indOps[i].value1, lhs, rhs1, 0);
			} else {
				expectLiteral(",");
				if(matchLiteral("#")) {
					consume();
					int imm = std::atoi(next().text.c_str());
					expect(AsmTokenizer::TypeNumber);
					expectLiteral("]");
					instr = VM::Instruction::makeTwoAddr(indOps[i].value1, lhs, rhs1, imm);
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
					instr = VM::Instruction::makeThreeAddr(indOps[i].value2, lhs, rhs1, rhs2, imm);
				}
			}

			return true;
		}
	}

	return false;
}

bool AsmParser::parseImmInstr(VM::Instruction &instr)
{
	static const NameTwoInt imm23Ops[] = {
		{ "add", VM::TwoAddrAddImm, VM::ThreeAddrAdd },
		{ "mult", VM::TwoAddrMultImm, VM::ThreeAddrMult },
	};

	for(int i=0; i<N_ENTRIES(imm23Ops); i++) {
		if(next().text == imm23Ops[i].name) {
			consume();
			int lhs = parseReg();
			expectLiteral(",");
			int rhs1 = parseReg();
			expectLiteral(",");

			if(matchLiteral("#")) {
				consume();
				int imm = std::atoi(next().text.c_str());
				expect(AsmTokenizer::TypeNumber);
				instr = VM::Instruction::makeTwoAddr(imm23Ops[i].value1, lhs, rhs1, imm);
			} else {
				int rhs2 = parseReg();
				instr = VM::Instruction::makeThreeAddr(imm23Ops[i].value2, lhs, rhs1, rhs2, 0);
			}
			return true;
		}
	}

	static const NameTwoInt imm12Ops[] = {
		{ "mov", VM::OneAddrLoadImm, VM::TwoAddrAddImm }
	};

	for(int i=0; i<N_ENTRIES(imm12Ops); i++) {
		if(next().text == imm12Ops[i].name) {
			consume();
			int lhs = parseReg();
			expectLiteral(",");

			if(matchLiteral("#")) {
				consume();
				int imm = std::atoi(next().text.c_str());
				expect(AsmTokenizer::TypeNumber);
				instr = VM::Instruction::makeOneAddr(imm12Ops[i].value1, lhs, imm);
			} else {
				int rhs = parseReg();
				instr = VM::Instruction::makeTwoAddr(imm12Ops[i].value2, lhs, rhs, 0);
			}
			return true;
		}
	}

	return false;
}

bool AsmParser::parseMultInstr(VM::Instruction &instr)
{
	static const NameTwoInt multOps[] = {
		{ "stm", VM::MultRegStore, 0 },
		{ "ldm", VM::MultRegLoad, 0 },
	};

	for(int i=0; i<N_ENTRIES(multOps); i++) {
		if(next().text == multOps[i].name) {
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

			instr = VM::Instruction::makeMultReg(multOps[i].value1, lhs, regs);
			return true;
		}
	}

	return false;
}

bool AsmParser::parseJumpCall(VM::Instruction &instr, int offset, std::map<int, std::string> &jumps)
{
	if(next().text == "jmp") {
		consume();
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegPC, 0);
		jumps[offset] = target;
		return true;
	}

	if(next().text == "cjmp") {
		consume();
		int pred = parseReg();
		expectLiteral(",");
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddCond, VM::RegPC, pred, VM::RegPC, 0);
		jumps[offset] = target;
		return true;
	}

	if(next().text == "ncjmp") {
		consume();
		int pred = parseReg();
		expectLiteral(",");
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddNCond, VM::RegPC, pred, VM::RegPC, 0);
		jumps[offset] = target;
		return true;
	}

	if(next().text == "call") {
		consume();
		std::string target = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		instr = VM::Instruction::makeOneAddr(VM::OneAddrCall, VM::RegPC, 0);
		jumps[offset] = target;
		return true;
	}

	return false;
}

bool AsmParser::parseStringLoad(VM::Instruction &instr, int offset, std::map<int, std::string> &stringLoads)
{
	if(matchLiteral("lea")) {
		consume();
		int lhs = parseReg();
		expectLiteral(",");
		std::string name = next().text;
		expect(AsmTokenizer::TypeIdentifier);
		stringLoads[offset] = name;
		instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, lhs, VM::RegPC, 0);
		return true;
	}

	return false;
}

int AsmParser::parseReg()
{
	NameInt regs[] = { { "sp", 13 }, { "lr", 14 }, { "pc", 15 } };

	const std::string &text = next().text;

	if(text[0] == 'r') {
		std::string num = text.substr(1);
		consume();
		return std::atoi(num.c_str());
	}

	for(int i=0; i<N_ENTRIES(regs); i++) {
		if(regs[i].name == text) {
			consume();
			return regs[i].value;
		}
	}

	errorExpected("register");
	return -1;
}

}