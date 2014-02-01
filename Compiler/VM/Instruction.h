#ifndef VM_INSTRUCTION_H
#define VM_INSTRUCTION_H

#include <iostream>

namespace VM {
	/*!
	 * \brief Instruction format for the VM
	 */
	struct Instruction {
		unsigned char type : 4; //!< Type of instruction

		/*!
		 * \brief Union of possible instruction bodies
		 */
		union {
			/*!
			 * \brief Two-address instruction form
			 */
			struct {
				unsigned char type : 4; //!< Type of two-address instruction
				unsigned char regLhs : 4; //!< Destination register
				unsigned char regRhs : 4; //!< Source register
				short imm : 16; //!< Immediate constant
			} two;

			/*!
			 * \brief Three-address instruction form
			 */
			struct {
				unsigned char type : 4; //!< Type of three-address instruction
				unsigned char regLhs : 4; //!< Destination register
				unsigned char regRhs1 : 4; //!< First source register
				unsigned char regRhs2 : 4; //!< Second source register
				long imm : 12; //!< Immediate constant
			} three;

			/*!
			 * \brief One-address instruction form
			 */
			struct {
				unsigned char type : 4; //!< Type of one-address instruction
				unsigned char reg : 4; //!< Register
				long imm : 20; //!< Immediate constant
			} one;

			/*!
			 * \brief Multiple-address instruction form
			 */
			struct {
				unsigned char type : 4; //!< Type of multiple-address instruction
				unsigned long regs : 16; //!< Bitmask of registers
			} mult;
		} u;

		static Instruction makeOneAddr(unsigned char type, unsigned char reg, long imm);
		static Instruction makeTwoAddr(unsigned char type, unsigned char regLhs, unsigned char regRhs, long imm);
		static Instruction makeThreeAddr(unsigned char type, unsigned char regLhs, unsigned char regRhs1, unsigned char regRhs2, short imm);
		static Instruction makeMultReg(unsigned char type, unsigned long regs);
	};

	std::ostream &operator<<(std::ostream &o, const Instruction &instr);

	const int InstrTwoAddr = 0x0; //!< Two-address instruction type
	const int InstrThreeAddr = 0x1; //!< Three-address instruction type
	const int InstrOneAddr = 0x2; //!< One-address instruction type
	const int InstrMultReg = 0x3; //!< Multiple-register instruction type

	const int TwoAddrAddImm = 0x0; //!< Add immediate to register
	const int TwoAddrMultImm = 0x1; //!< Multiply immediate with a register
	const int TwoAddrLoad = 0x2; //!< Load register from stack
	const int TwoAddrStore = 0x3; //!< Store register to stack
	const int TwoAddrNew = 0x4; //!< Allocate new memory

	const int ThreeAddrAdd = 0x0; //!< Add two registers
	const int ThreeAddrSub = 0x1; //!< Subtract two registers
	const int ThreeAddrMult = 0x2; //!< Multiply two registers
	const int ThreeAddrAddCond = 0x3; //!< Add immediate to src2 if src1 is nonzero
	const int ThreeAddrEqual = 0x4; //!< Compare two registers for equality
	const int ThreeAddrNEqual = 0x5; //!< Compare two registers for inequality
	const int ThreeAddrLessThan = 0x6; //!< Compare two registers for less-than
	const int ThreeAddrLessThanE = 0x7; //!< Compare two registers for less-than-equal
	const int ThreeAddrGreaterThan = 0x8; //!< Compare two registers for greater-than
	const int ThreeAddrGreaterThanE = 0x9; //!< Compare two registers for greater-than-equal
	const int ThreeAddrLoad = 0xa; //!< Load register from memory
	const int ThreeAddrStore = 0xb; //!< Store register into memory

	const int OneAddrLoadImm = 0x0; //!< Load constant into register
	const int OneAddrPrint = 0x1; //!< Print register
	const int OneAddrCall = 0x2; //!< Call procedure: Save next address into LR and jump to location in register

	const int MultRegStore = 0x0; //!< Store multiple registers to stack
	const int MultRegLoad = 0x1; //!< Load multiple registers from stack

	const int RegPC = 0xf; // Program Counter register
	const int RegLR = 0xe; // Link Register: receives return address during procedure calls
	const int RegSP = 0xd; // Stack Pointer
}
#endif