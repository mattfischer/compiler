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

	static void printStd(std::ostream &o, const std::string &name, int reg0 = -1, int reg1 = -1, int reg2 = -1)
	{
		o << name << " ";
		if(reg0 != -1) {
			o << regName(reg0);
		}

		if(reg1 != -1) {
			o << ", " << regName(reg1);
		}

		if(reg2 != -1) {
			o << ", " << regName(reg2);
		}
	}

	static void printImm(std::ostream &o, const std::string &name, int reg0, int reg1, int imm)
	{
		bool needComma = false;
		o << name << " ";
		if(reg0 != -1) {
			o << regName(reg0);
			needComma = true;
		}

		if(reg1 != -1) {
			if(needComma) {
				o << ", ";
			}
			o << regName(reg1);
			needComma = true;
		}

		if(needComma) {
			o << ", ";
		}
		o << "#" << imm;
	}

	static void printInd(std::ostream &o, const std::string &name, int reg0, int reg1, int reg2, int imm = 0)
	{
		o << name << " ";
		if(reg0 != -1) {
			o << regName(reg0) << ", ";
		}

		o << "[" << regName(reg1);
		if(reg2 != -1) {
			o << ", " << regName(reg2);
		}

		if(imm != 0) {
			o << ", #" << imm;
		}
		o << "]";
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
				if(instr.u.two.regLhs == RegPC) {
					// Adds to PC should be printed as a jump
					if(instr.u.two.imm == 0) {
						printStd(o, "jmp", instr.u.two.regRhs);
					} else {
						if(instr.u.two.regRhs == RegPC) {
							// PC-relative jumps can be printed with their constant only
							printImm(o, "jmp", -1, -1, instr.u.two.imm);
						} else {
							printImm(o, "jmp", instr.u.two.regRhs, -1, instr.u.two.imm);
						}
					}
				} else {
					if(instr.u.two.imm > 0) {
						// Print add with constant
						printImm(o, "add", instr.u.two.regLhs, instr.u.two.regRhs, instr.u.two.imm);
					} else if(instr.u.two.imm == 0){
						// With no immediate value, print as a move
						printStd(o, "mov", instr.u.two.regLhs, instr.u.two.regRhs);
					} else {
						// With a negative constant, print as a subtract
						printImm(o, "sub", instr.u.two.regLhs, instr.u.two.regRhs, -instr.u.two.imm);
					}
				}
				break;

			case TwoAddrMultImm:
				printImm(o, "mult", instr.u.two.regLhs, instr.u.two.regRhs, instr.u.two.imm);
				break;

			case TwoAddrLoad:
				printInd(o, "ldr", instr.u.two.regLhs, instr.u.two.regRhs, -1, instr.u.two.imm);
				break;

			case TwoAddrStore:
				printInd(o, "str", instr.u.two.regLhs, instr.u.two.regRhs, -1, instr.u.two.imm);
				break;

			case TwoAddrNew:
				printStd(o, "new", instr.u.two.regLhs, instr.u.two.regRhs);
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
				printStd(o, "add", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrSub:
				printStd(o, "sub", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrMult:
				printStd(o, "mult", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrAddCond:
				if(instr.u.three.regLhs == RegPC) {
					// If target register is PC, print as a conditional jump
					if(instr.u.three.imm == 0) {
						printStd(o, "cjmp", instr.u.three.regRhs1, instr.u.three.regRhs2);
					} else {
						if(instr.u.three.regRhs2 == RegPC) {
							// PC-relative jumps can be printed with their constant only
							printImm(o, "cjmp", instr.u.three.regRhs1, -1, instr.u.three.imm);
						} else {
							printImm(o, "cjmp", instr.u.three.regRhs1, instr.u.three.regRhs2, instr.u.three.imm);
						}
					}
				} else {
					if(instr.u.three.imm == 0) {
						// With no immediate value, print as a conditional move
						printStd(o, "cmov", instr.u.three.regRhs1, instr.u.three.regRhs2);
					} else {
						// With a positive immediate value, print as a conditional add
						o << "cadd " << regName(instr.u.three.regRhs1) << ", " << regName(instr.u.three.regLhs) << ", " << regName(instr.u.three.regRhs2) << ", #" << instr.u.three.imm;
					}
				}
				break;

			case ThreeAddrEqual:
				printStd(o, "equ", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrNEqual:
				printStd(o, "neq", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrLessThan:
				printStd(o, "lt", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrLessThanE:
				printStd(o, "lte", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrGreaterThan:
				printStd(o, "gt", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrGreaterThanE:
				printStd(o, "gte", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrOr:
				printStd(o, "or", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrAnd:
				printStd(o, "and", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrLoad:
				printInd(o, "ldr", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
				break;

			case ThreeAddrStore:
				printInd(o, "str", instr.u.three.regLhs, instr.u.three.regRhs1, instr.u.three.regRhs2);
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
				printImm(o, "mov", instr.u.one.reg, -1, instr.u.one.imm);
				break;

			case OneAddrPrint:
				printStd(o, "print", instr.u.one.reg);
				break;

			case OneAddrCall:
				printInd(o, "call", -1, instr.u.one.reg, instr.u.one.imm);
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
				if(firstReg != i-1) {
					o << regName(firstReg) << "-";
				}
				o << regName(i-1);
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
	 * \param regLhs Destination register
	 * \param regRhs Source register
	 * \param imm Immediate constant
	 * \return Instruction
	 */
	Instruction Instruction::makeTwoAddr(unsigned char type, unsigned char regLhs, unsigned char regRhs, long imm)
	{
		Instruction instr;

		instr.type = InstrTwoAddr;
		instr.u.two.type = type;
		instr.u.two.regLhs = regLhs;
		instr.u.two.regRhs = regRhs;
		instr.u.two.imm = imm;

		return instr;
	}

	/*!
	 * \brief Construct a three-address form instruction
	 * \param type Type of three-address instruction
	 * \param regLhs Destination register
	 * \param regRhs1 Source register 1
	 * \param regRhs2 Source register 2
	 * \param imm Immediate constant
	 * \return Instruction
	 */
	Instruction Instruction::makeThreeAddr(unsigned char type, unsigned char regLhs, unsigned char regRhs1, unsigned char regRhs2, short imm)
	{
		Instruction instr;

		instr.type = InstrThreeAddr;
		instr.u.three.type = type;
		instr.u.three.regLhs = regLhs;
		instr.u.three.regRhs1 = regRhs1;
		instr.u.three.regRhs2 = regRhs2;
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