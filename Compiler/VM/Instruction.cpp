#include "VM/Instruction.h"

#include <stdio.h>

#include <sstream>

namespace VM {
	std::string regName(int reg)
	{
		switch(reg) {
			case VM::RegSP:
				return "sp";

			case VM::RegLR:
				return "lr";

			case VM::RegPC:
				return "pc";

			default:
				{
					std::stringstream s;
					s << "r" << reg;
					return s.str();
				}
		}
	}

	static void printTwoAddr(std::ostream &o, const Instruction &instr)
	{
		switch(instr.u.two.type) {
			case TwoAddrAddImm:
				if(instr.u.two.regDst == RegPC) {
					if(instr.u.two.imm == 0) {
						o << "jmp " << regName(instr.u.two.regSrc);
					} else {
						if(instr.u.two.regSrc == RegPC) {
							o << "jmp #" << int(instr.u.two.imm);
						} else {
							o << "jmp " << regName(instr.u.two.regSrc) << ", #" << int(instr.u.two.imm);
						}
					}
				} else {
					if(instr.u.two.imm > 0) {
						o << "add " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc) << ", #" << int(instr.u.two.imm);
					} else if(instr.u.two.imm == 0){
						o << "mov " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc);
					} else {
						o << "sub " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc) << ", #" << -int(instr.u.two.imm);
					}
				}
				break;

			case TwoAddrLoad:
				if(instr.u.two.imm == 0) {
					o << "ldr " << regName(instr.u.two.regDst) << ", [" << regName(instr.u.two.regSrc) << "]";
				} else {
					o << "ldr " << regName(instr.u.two.regDst) << ", [" << regName(instr.u.two.regSrc) << ", # " << int(instr.u.two.imm) << "]";
				}
				break;

			case TwoAddrStore:
				if(instr.u.two.imm == 0) {
					o << "str " << regName(instr.u.two.regSrc) << ", [" << regName(instr.u.two.regDst) << "]";
				} else {
					o << "str " << regName(instr.u.two.regSrc) << ", [" << regName(instr.u.two.regDst) << ", #" << int(instr.u.two.imm) << "]";
				}
				break;

		}
	}

	static void printThreeAddr(std::ostream &o, const Instruction &instr)
	{
		switch(instr.u.three.type) {
			case ThreeAddrAdd:
				o << "add " << regName(instr.u.three.regDst) << ", " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2);
				break;

			case ThreeAddrMult:
				o << "mult " << regName(instr.u.three.regDst) << ", " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2);
				break;

			case ThreeAddrAddCond:
				if(instr.u.three.regDst == RegPC) {
					if(instr.u.three.imm == 0) {
						o << "cjmp " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2);
					} else {
						if(instr.u.three.regSrc2 == RegPC) {
							o << "cjmp " << regName(instr.u.three.regSrc1) << ", #" << int(instr.u.three.imm);
						} else {
							o << "cjmp " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2) << ", #" << int(instr.u.three.imm);
						}
					}
				} else {
					if(instr.u.three.imm == 0) {
						o << "cmov " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regDst) << ", " << regName(instr.u.three.regSrc2);
					} else {
						o << "cadd " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regDst) << ", " << regName(instr.u.three.regSrc2) << ", #" << int(instr.u.three.imm);
					}
				}
				break;

			case ThreeAddrEqual:
				o << "equ " << regName(instr.u.three.regDst) << ", " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2);
				break;

			case ThreeAddrNEqual:
				o << "neq " << regName(instr.u.three.regDst) << ", " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2);
				break;
		}
	}

	static void printOneAddr(std::ostream &o, const Instruction &instr)
	{
		switch(instr.u.one.type) {
			case OneAddrLoadImm:
				o << "mov " << regName(instr.u.one.reg) << ", #" << int(instr.u.one.imm);
				break;

			case OneAddrPrint:
				o << "print " << regName(instr.u.one.reg);
				break;

			case OneAddrCall:
				o << "call [" << regName(instr.u.one.reg) << ", #" << int(instr.u.one.imm) << "]";
				break;
		}
	}

	static void printMultReg(std::ostream &o, const Instruction &instr)
	{
		switch(instr.u.mult.type) {
			case MultRegLoad:
				o << "ldm ";
				break;

			case MultRegStore:
				o << "stm ";
				break;
		}

		bool needComma = false;
		for(int i=0; i<16; i++) {
			if(instr.u.mult.regs & (1 << i)) {
				if(needComma) {
					o << ", ";
				}
				o << regName(i);
				needComma = true;
			}
		}
	}

	std::ostream &operator<<(std::ostream &o, const Instruction &instr)
	{
		switch(instr.type) {
			case InstrOneAddr:
				printOneAddr(o, instr);
				break;

			case InstrTwoAddr:
				printTwoAddr(o, instr);
				break;

			case InstrThreeAddr:
				printThreeAddr(o, instr);
				break;

			case InstrMultReg:
				printMultReg(o, instr);
				break;
		}

		return o;
	}

	Instruction Instruction::makeOneAddr(unsigned char type, unsigned char reg, long imm)
	{
		Instruction instr;

		instr.type = InstrOneAddr;
		instr.u.one.type = type;
		instr.u.one.reg = reg;
		instr.u.one.imm = imm;

		return instr;
	}

	Instruction Instruction::makeTwoAddr(unsigned char type, unsigned char regDst, unsigned char regSrc, long imm)
	{
		Instruction instr;

		instr.type = InstrTwoAddr;
		instr.u.two.type = type;
		instr.u.two.regDst = regDst;
		instr.u.two.regSrc = regSrc;
		instr.u.two.imm = imm;

		return instr;
	}

	Instruction Instruction::makeThreeAddr(unsigned char type, unsigned char regDst, unsigned char regSrc1, unsigned char regSrc2, short imm)
	{
		Instruction instr;

		instr.type = InstrThreeAddr;
		instr.u.three.type = type;
		instr.u.three.regDst = regDst;
		instr.u.three.regSrc1 = regSrc1;
		instr.u.three.regSrc2 = regSrc2;
		instr.u.three.imm = imm;

		return instr;
	}

	Instruction Instruction::makeMultReg(unsigned char type, unsigned long regs)
	{
		Instruction instr;

		instr.type = InstrMultReg;
		instr.u.mult.type = type;
		instr.u.mult.regs = regs;

		return instr;
	}
}