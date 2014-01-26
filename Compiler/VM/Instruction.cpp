#include "VM/Instruction.h"

#include <sstream>

namespace VM {
	/*!
	 * \brief Return name of register
	 * \param reg Register number
	 * \return Register name
	 */
	std::string regName(int reg)
	{
		// Handle specially-named registers first
		switch(reg) {
			case VM::RegSP:
				return "sp";

			case VM::RegLR:
				return "lr";

			case VM::RegPC:
				return "pc";

			default:
				{
					// Otherwise, construct rX name
					std::stringstream s;
					s << "r" << reg;
					return s.str();
				}
		}
	}

	/*!
	 * \brief Print a two-address instruction
	 * \param o Output stream
	 * \param instr Instruction to print
	 */
	static void printTwoAddr(std::ostream &o, const Instruction &instr)
	{
		switch(instr.u.two.type) {
			case TwoAddrAddImm:
				if(instr.u.two.regDst == RegPC) {
					// Adds to PC should be printed as a jump
					if(instr.u.two.imm == 0) {
						o << "jmp " << regName(instr.u.two.regSrc);
					} else {
						if(instr.u.two.regSrc == RegPC) {
							// PC-relative jumps can be printed with their constant only
							o << "jmp #" << int(instr.u.two.imm);
						} else {
							o << "jmp " << regName(instr.u.two.regSrc) << ", #" << int(instr.u.two.imm);
						}
					}
				} else {
					if(instr.u.two.imm > 0) {
						// Print add with constant
						o << "add " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc) << ", #" << int(instr.u.two.imm);
					} else if(instr.u.two.imm == 0){
						// With no immediate value, print as a move
						o << "mov " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc);
					} else {
						// With a negative constant, print as a subtract
						o << "sub " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc) << ", #" << -int(instr.u.two.imm);
					}
				}
				break;

			case TwoAddrMultImm:
				o << "mult " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc) << ", #" << int(instr.u.two.imm);
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

			case TwoAddrNew:
				o << "new " << regName(instr.u.two.regDst) << ", " << regName(instr.u.two.regSrc);
				break;
		}
	}

	/*!
	 * \brief Print a three-address instruction
	 * \param o Output stream
	 * \param instr Instruction to print
	 */
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
					// If target register is PC, print as a conditional jump
					if(instr.u.three.imm == 0) {
						o << "cjmp " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2);
					} else {
						if(instr.u.three.regSrc2 == RegPC) {
							// PC-relative jumps can be printed with their constant only
							o << "cjmp " << regName(instr.u.three.regSrc1) << ", #" << int(instr.u.three.imm);
						} else {
							o << "cjmp " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regSrc2) << ", #" << int(instr.u.three.imm);
						}
					}
				} else {
					if(instr.u.three.imm == 0) {
						// With no immediate value, print as a conditional move
						o << "cmov " << regName(instr.u.three.regSrc1) << ", " << regName(instr.u.three.regDst) << ", " << regName(instr.u.three.regSrc2);
					} else {
						// With a positive immediate value, print as a conditional add
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

	/*!
	 * \brief Print a one-address instruction
	 * \param o Output stream
	 * \param instr Instruction to print
	 */
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

	/*!
	 * \brief Print a multiple-register instruction
	 * \param o Output stream
	 * \param instr Instruction to print
	 */
	static void printMultReg(std::ostream &o, const Instruction &instr)
	{
		// Print the instruction name
		switch(instr.u.mult.type) {
			case MultRegLoad:
				o << "ldm ";
				break;

			case MultRegStore:
				o << "stm ";
				break;
		}

		// Print the register list
		o << "{";
		bool needComma = false;
		int firstReg = -1;
		for(int i=0; i<16; i++) {
			if(firstReg != -1 && ((instr.u.mult.regs & (1 << i)) == 0 || i == VM::RegSP)) {
				if(needComma) {
					o << ", ";
				}
				o << regName(firstReg) << "-" << regName(i-1);
				needComma = true;
				firstReg = -1;
			}

			if(instr.u.mult.regs & (1 << i)) {
				if(i < VM::RegSP) {
					if(firstReg == -1) {
						firstReg = i;
					}
				} else {
					if(needComma) {
						o << ", ";
					}
					o << regName(i);
					needComma = true;
				}
			}
		}
		o << "}";
	}

	/*!
	 * \brief Stream output operator for instructions
	 * \param o Output stream
	 * \param instr Instruction
	 * \return Output stream
	 */
	std::ostream &operator<<(std::ostream &o, const Instruction &instr)
	{
		// Dispatch to appropriate print routine
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

	/*!
	 * \brief Construct a one-address form instruction
	 * \param type Type of one-address instruction
	 * \param reg Register
	 * \param imm Immediate constant
	 * \return Instruction
	 */
	Instruction Instruction::makeOneAddr(unsigned char type, unsigned char reg, long imm)
	{
		Instruction instr;

		instr.type = InstrOneAddr;
		instr.u.one.type = type;
		instr.u.one.reg = reg;
		instr.u.one.imm = imm;

		return instr;
	}

	/*!
	 * \brief Construct a two-address form instruction
	 * \param type Type of two-address instruction
	 * \param regDst Destination register
	 * \param regSrc Source register
	 * \param imm Immediate constant
	 * \return Instruction
	 */
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

	/*!
	 * \brief Construct a three-address form instruction
	 * \param type Type of three-address instruction
	 * \param regDst Destination register
	 * \param regSrc1 Source register 1
	 * \param regSrc2 Source register 2
	 * \param imm Immediate constant
	 * \return Instruction
	 */
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

	/*!
	 * \brief Construct a multiple-register form instruction
	 * \param type Type of multiple-register instruction
	 * \param regs Bitmask of registers
	 * \return Instruction
	 */
	Instruction Instruction::makeMultReg(unsigned char type, unsigned long regs)
	{
		Instruction instr;

		instr.type = InstrMultReg;
		instr.u.mult.type = type;
		instr.u.mult.regs = regs;

		return instr;
	}
}