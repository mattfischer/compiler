#include "VM/Instruction.h"

#include <stdio.h>

namespace VM {
	static void printTwoAddr(Instruction instr)
	{
		switch(instr.u.two.type) {
			case TwoAddrAddImm:
				if(instr.u.two.regDst == RegPC) {
					if(instr.u.two.imm == 0) {
						printf("jmp r%i", instr.u.two.regSrc);
					} else {
						if(instr.u.two.regSrc == RegPC) {
							printf("jmp #%i", instr.u.two.imm);
						} else {
							printf("jmp r%i, #%i", instr.u.two.regSrc, instr.u.two.imm);
						}
					}
				} else {
					if(instr.u.two.imm == 0) {
						printf("mov r%i, r%i", instr.u.two.regDst, instr.u.two.regSrc);
					} else {
						printf("add r%i, r%i, #%i", instr.u.two.regDst, instr.u.two.regSrc, instr.u.two.imm);
					}
				}
				break;

			case TwoAddrLoad:
				if(instr.u.two.imm == 0) {
					printf("ldr r%i, [r%i]", instr.u.two.regDst, instr.u.two.regSrc);
				} else {
					printf("ldr r%i, [r%i, #%i]", instr.u.two.regDst, instr.u.two.regSrc, instr.u.two.imm);
				}
				break;

			case TwoAddrStore:
				if(instr.u.two.imm == 0) {
					printf("str r%i, [r%i]", instr.u.two.regSrc, instr.u.two.regDst);
				} else {
					printf("str r%i, [r%i, #%i]", instr.u.two.regSrc, instr.u.two.regDst, instr.u.two.imm);
				}
				break;

		}
	}

	static void printThreeAddr(Instruction instr)
	{
		switch(instr.u.three.type) {
			case ThreeAddrAdd:
				printf("add r%i, r%i, r%i", instr.u.three.regDst, instr.u.three.regSrc1, instr.u.three.regSrc2);
				break;

			case ThreeAddrMult:
				printf("mult r%i, r%i, r%i", instr.u.three.regDst, instr.u.three.regSrc1, instr.u.three.regSrc2);
				break;

			case ThreeAddrAddCond:
				if(instr.u.three.regDst == RegPC) {
					if(instr.u.three.imm == 0) {
						printf("cjmp r%i, r%i", instr.u.three.regSrc1, instr.u.three.regSrc2);
					} else {
						if(instr.u.three.regSrc2 == RegPC) {
							printf("cjmp r%i, #%i", instr.u.three.regSrc1, instr.u.three.imm);
						} else {
							printf("cjmp r%i, r%i, #%i", instr.u.three.regSrc1, instr.u.three.regSrc2, instr.u.three.imm);
						}
					}
				} else {
					if(instr.u.three.imm == 0) {
						printf("cmov r%i, r%i, r%i", instr.u.three.regSrc1, instr.u.three.regDst, instr.u.three.regSrc2);
					} else {
						printf("cadd r%i, r%i, r%i, #%i", instr.u.three.regSrc1, instr.u.three.regDst, instr.u.three.regSrc2, instr.u.three.imm);
					}
				}
				break;

			case ThreeAddrEqual:
				printf("equ r%i, r%i, r%i", instr.u.three.regDst, instr.u.three.regSrc1, instr.u.three.regSrc2);
				break;

			case ThreeAddrNEqual:
				printf("neq r%i, r%i, r%i", instr.u.three.regDst, instr.u.three.regSrc1, instr.u.three.regSrc2);
				break;
		}
	}

	static void printOneAddr(Instruction instr)
	{
		switch(instr.u.one.type) {
			case OneAddrLoadImm:
				printf("mov r%i, #%i", instr.u.one.reg, instr.u.one.imm);
				break;

			case OneAddrPrint:
				printf("print r%i", instr.u.one.reg);
				break;

			case OneAddrCall:
				printf("call [r%i, #%i]", instr.u.one.reg, instr.u.one.imm);
				break;
		}
	}

	void Instruction::print()
	{
		switch(type) {
			case InstrOneAddr:
				printOneAddr(*this);
				break;

			case InstrTwoAddr:
				printTwoAddr(*this);
				break;

			case InstrThreeAddr:
				printThreeAddr(*this);
				break;
		}
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
}