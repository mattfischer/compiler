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
		switch(instr.two.type) {
			case TwoAddrAddImm:
				if(instr.two.regLhs == RegPC) {
					// Adds to PC should be printed as a jump
					if(instr.two.imm == 0) {
						printStd(o, "jmp", instr.two.regRhs);
					} else {
						if(instr.two.regRhs == RegPC) {
							// PC-relative jumps can be printed with their constant only
							printImm(o, "jmp", -1, -1, instr.two.imm);
						} else {
							printImm(o, "jmp", instr.two.regRhs, -1, instr.two.imm);
						}
					}
				} else {
					if(instr.two.imm > 0) {
						// Print add with constant
						printImm(o, "add", instr.two.regLhs, instr.two.regRhs, instr.two.imm);
					} else if(instr.two.imm == 0){
						// With no immediate value, print as a move
						printStd(o, "mov", instr.two.regLhs, instr.two.regRhs);
					} else {
						// With a negative constant, print as a subtract
						printImm(o, "sub", instr.two.regLhs, instr.two.regRhs, -instr.two.imm);
					}
				}
				break;

			case TwoAddrMultImm:
				printImm(o, "mult", instr.two.regLhs, instr.two.regRhs, instr.two.imm);
				break;

			case TwoAddrLoad:
				printInd(o, "ldr", instr.two.regLhs, instr.two.regRhs, -1, instr.two.imm);
				break;

			case TwoAddrStore:
				printInd(o, "str", instr.two.regLhs, instr.two.regRhs, -1, instr.two.imm);
				break;

			case TwoAddrNew:
				printStd(o, "new", instr.two.regLhs, instr.two.regRhs);
				break;

			case TwoAddrStringBool:
				printStd(o, "strbool", instr.two.regLhs, instr.two.regRhs);
				break;

			case TwoAddrStringInt:
				printStd(o, "strint", instr.two.regLhs, instr.two.regRhs);
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
		switch(instr.three.type) {
			case ThreeAddrAdd:
				printStd(o, "add", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrSub:
				printStd(o, "sub", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrMult:
				printStd(o, "mult", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrAddCond:
				if(instr.three.regLhs == RegPC) {
					// If target register is PC, print as a conditional jump
					if(instr.three.imm == 0) {
						printStd(o, "cjmp", instr.three.regRhs1, instr.three.regRhs2);
					} else {
						if(instr.three.regRhs2 == RegPC) {
							// PC-relative jumps can be printed with their constant only
							printImm(o, "cjmp", instr.three.regRhs1, -1, instr.three.imm);
						} else {
							printImm(o, "cjmp", instr.three.regRhs1, instr.three.regRhs2, instr.three.imm);
						}
					}
				} else {
					if(instr.three.imm == 0) {
						// With no immediate value, print as a conditional move
						printStd(o, "cmov", instr.three.regRhs1, instr.three.regRhs2);
					} else {
						// With a positive immediate value, print as a conditional add
						o << "cadd " << regName(instr.three.regRhs1) << ", " << regName(instr.three.regLhs) << ", " << regName(instr.three.regRhs2) << ", #" << instr.three.imm;
					}
				}
				break;

			case ThreeAddrEqual:
				printStd(o, "equ", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrNEqual:
				printStd(o, "neq", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrLessThan:
				printStd(o, "lt", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrLessThanE:
				printStd(o, "lte", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrGreaterThan:
				printStd(o, "gt", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrGreaterThanE:
				printStd(o, "gte", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrOr:
				printStd(o, "or", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrAnd:
				printStd(o, "and", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
				break;

			case ThreeAddrLoad:
				printInd(o, "ldr", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2, instr.three.imm);
				break;

			case ThreeAddrStore:
				printInd(o, "str", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2, instr.three.imm);
				break;

			case ThreeAddrConcat:
				printStd(o, "concat", instr.three.regLhs, instr.three.regRhs1, instr.three.regRhs2);
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
		switch(instr.one.type) {
			case OneAddrLoadImm:
				printImm(o, "mov", instr.one.reg, -1, instr.one.imm);
				break;

			case OneAddrPrint:
				printStd(o, "print", instr.one.reg);
				break;

			case OneAddrCall:
				printInd(o, "call", -1, instr.one.reg, -1, instr.one.imm);
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
		switch(instr.mult.type) {
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
			if(firstReg != -1 && ((instr.mult.regs & (1 << i)) == 0 || i == VM::RegSP)) {
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

			if(instr.mult.regs & (1 << i)) {
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
		instr.one.type = type;
		instr.one.reg = reg;
		instr.one.imm = imm;

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
		instr.two.type = type;
		instr.two.regLhs = regLhs;
		instr.two.regRhs = regRhs;
		instr.two.imm = imm;

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
		instr.three.type = type;
		instr.three.regLhs = regLhs;
		instr.three.regRhs1 = regRhs1;
		instr.three.regRhs2 = regRhs2;
		instr.three.imm = imm;

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
		instr.mult.type = type;
		instr.mult.regs = regs;

		return instr;
	}
}