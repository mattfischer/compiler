#include "VM/Interp.h"

#include "VM/Heap.h"

#include <sstream>

namespace VM {
	static int concatStrings(unsigned char mem[], Heap &heap, int str1, int str2)
	{
		char *s1 = (char*)&mem[str1];
		char *s2 = (char*)&mem[str2];
		int l1 = strlen(s1);
		int l2 = strlen(s2);
		int result = heap.allocate(l1 + l2 + 1);
		char *r = (char*)&mem[result];
		std::strcpy(r, s1);
		std::strcpy(r + l1, s2);
		r[l1 + l2] = '\0';

		return result;
	}

	static int strBool(unsigned char mem[], Heap &heap, int b)
	{
		std::string str;
		if(b) {
			str = "true";
		} else {
			str = "false";
		}

		int result = heap.allocate(str.size() + 1);
		char *r = (char*)&mem[result];
		std::strcpy(r, str.c_str());
		r[str.size()] = '\0';
		return result;
	}

	static int strInt(unsigned char mem[], Heap &heap, int x)
	{
		std::stringstream s;
		s << x;
		std::string str = s.str();

		int result = heap.allocate(str.size() + 1);
		char *r = (char*)&mem[result];
		std::strcpy(r, str.c_str());
		r[str.size()] = '\0';
		return result;
	}

	/*!
	 * \brief Run a VM program
	 * \param program Program to run
	 */
	void Interp::run(const Program &program)
	{
		int regs[16];
		int curPC;
		unsigned char mem[1024];
		Heap heap(mem, 1024, (int)program.instructions.size());

		// Initialize all registers to 0
		memset(regs, 0, 16 * sizeof(int));

		memcpy(mem, &program.instructions[0], program.instructions.size());

		// Set SP to the top of the stack
		regs[VM::RegSP] = sizeof(mem);

		// Set LR to beyond the end of the program, so program exit can be detected
		regs[VM::RegLR] = 0xffffffff;

		// Set PC to the program entry point
		regs[VM::RegPC] = program.start;

		// Loop until PC is set beyond the end of the program
		while(regs[VM::RegPC] != 0xffffffff) {
			curPC = regs[VM::RegPC];
			Instruction instr;
			std::memcpy(&instr, &mem[regs[VM::RegPC]], 4);

			// Examine the instruction, and interpret it accordingly
			switch(instr.type) {
				case VM::InstrOneAddr:
					switch(instr.one.type) {
						case VM::OneAddrLoadImm:
							regs[instr.one.reg] = instr.one.imm;
							break;

						case VM::OneAddrPrint:
							for(int i = regs[instr.one.reg]; mem[i] != '\0'; i++) {
								std::cout << (char)mem[i];
							}
							std::cout << std::endl;
							break;

						case VM::OneAddrCall:
							regs[VM::RegLR] = regs[VM::RegPC] + 4;
							regs[VM::RegPC] = regs[instr.one.reg] + 4 * instr.one.imm;
							break;
					}
					break;

				case VM::InstrTwoAddr:
					switch(instr.two.type) {
						case VM::TwoAddrAddImm:
							regs[instr.two.regLhs] = regs[instr.two.regRhs] + instr.two.imm;
							break;

						case VM::TwoAddrMultImm:
							regs[instr.two.regLhs] = regs[instr.two.regRhs] * instr.two.imm;
							break;

						case VM::TwoAddrLoad:
							regs[instr.two.regLhs] = mem[regs[instr.two.regRhs] + instr.two.imm];
							break;

						case VM::TwoAddrStore:
							mem[regs[instr.two.regRhs] + instr.two.imm] = regs[instr.two.regLhs];
							break;

						case VM::TwoAddrNew:
							regs[instr.two.regLhs] = heap.allocate(regs[instr.two.regRhs]);
							break;

						case VM::TwoAddrStringBool:
							regs[instr.two.regLhs] = strBool(mem, heap, regs[instr.two.regRhs]);
							break;

						case VM::TwoAddrStringInt:
							regs[instr.two.regLhs] = strInt(mem, heap, regs[instr.two.regRhs]);
							break;
					}
					break;

				case VM::InstrThreeAddr:
					switch(instr.three.type) {
						case VM::ThreeAddrAdd:
							regs[instr.three.regLhs] = regs[instr.three.regRhs1] + regs[instr.three.regRhs2];
							break;

						case VM::ThreeAddrSub:
							regs[instr.three.regLhs] = regs[instr.three.regRhs1] - regs[instr.three.regRhs2];
							break;

						case VM::ThreeAddrMult:
							regs[instr.three.regLhs] = regs[instr.three.regRhs1] * regs[instr.three.regRhs2];
							break;

						case VM::ThreeAddrAddCond:
							if(regs[instr.three.regRhs1]) {
								regs[instr.three.regLhs] = regs[instr.three.regRhs2] + instr.three.imm;
							}
							break;

						case VM::ThreeAddrEqual:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] == regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrNEqual:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] != regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrLessThan:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] < regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrLessThanE:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] <= regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrGreaterThan:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] > regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrGreaterThanE:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] >= regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrOr:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] || regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrAnd:
							regs[instr.three.regLhs] = (regs[instr.three.regRhs1] && regs[instr.three.regRhs2]);
							break;

						case VM::ThreeAddrLoad:
							regs[instr.three.regLhs] = *(int*)(mem + regs[instr.three.regRhs1] + (regs[instr.three.regRhs2] << instr.three.imm));
							break;

						case VM::ThreeAddrStore:
							*(int*)(mem + regs[instr.three.regRhs1] + (regs[instr.three.regRhs2] << instr.three.imm)) = regs[instr.three.regLhs];
							break;

						case VM::ThreeAddrConcat:
							regs[instr.three.regLhs] = concatStrings(mem, heap, regs[instr.three.regRhs1], regs[instr.three.regRhs2]);
							break;
					}
					break;

				case VM::InstrMultReg:
					switch(instr.mult.type) {
						case VM::MultRegLoad:
							for(int i=0; i<16; i++) {
								if(instr.mult.regs & (1 << i)) {
									regs[i] = *(int*)(mem + regs[VM::RegSP]);
									regs[VM::RegSP] += sizeof(int);
								}
							}
							break;

						case VM::MultRegStore:
							for(int i=15; i>=0; i--) {
								if(instr.mult.regs & (1 << i)) {
									regs[VM::RegSP] -= sizeof(int);
									*(int*)(mem + regs[VM::RegSP]) = regs[i];
								}
							}
							break;
					}
					break;
			}

			// If PC was not explicitly set by the instruction, increment it to the next instruction
			if(regs[VM::RegPC] == curPC) {
				regs[VM::RegPC] += 4;
			}
		}
	}
}