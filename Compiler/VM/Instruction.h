#ifndef VM_INSTRUCTION_H
#define VM_INSTRUCTION_H

#include <iostream>

namespace VM {
	struct Instruction {
		unsigned char type : 4;

		union {
			struct {
				unsigned char type : 4;
				unsigned char regDst : 4;
				unsigned char regSrc : 4;
				short imm : 16;
			} two;

			struct {
				unsigned char type : 4;
				unsigned char regDst : 4;
				unsigned char regSrc1 : 4;
				unsigned char regSrc2 : 4;
				long imm : 12;
			} three;

			struct {
				unsigned char type : 4;
				unsigned char reg : 4;
				long imm : 20;
			} one;

			struct {
				unsigned char type : 4;
				unsigned long regs : 16;
			} mult;
		} u;

		static Instruction makeOneAddr(unsigned char type, unsigned char reg, long imm);
		static Instruction makeTwoAddr(unsigned char type, unsigned char regDst, unsigned char regSrc, long imm);
		static Instruction makeThreeAddr(unsigned char type, unsigned char regDst, unsigned char regSrc1, unsigned char regSrc2, short imm);
		static Instruction makeMultReg(unsigned char type, unsigned long regs);
	};

	std::ostream &operator<<(std::ostream &o, const Instruction &instr);

	const int InstrTwoAddr = 0x0;
	const int InstrThreeAddr = 0x1;
	const int InstrOneAddr = 0x2;
	const int InstrMultReg = 0x3;

	const int TwoAddrAddImm = 0x0;
	const int TwoAddrLoad = 0x1;
	const int TwoAddrStore = 0x2;

	const int ThreeAddrAdd = 0x0;
	const int ThreeAddrMult = 0x1;
	const int ThreeAddrAddCond = 0x2;
	const int ThreeAddrEqual = 0x3;
	const int ThreeAddrNEqual = 0x4;

	const int OneAddrLoadImm = 0x0;
	const int OneAddrPrint = 0x1;
	const int OneAddrCall = 0x2;

	const int MultRegStore = 0x0;
	const int MultRegLoad = 0x1;

	const int RegPC = 0xf;
	const int RegLR = 0xe;
	const int RegSP = 0xd;
}
#endif