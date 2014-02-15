#include "VM/Interp.h"

#include "VM/Heap.h"

namespace VM {
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
		regs[VM::RegSP] = sizeof(mem) - 1;

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

						case VM::OneAddrPrintInt:
							std::cout << int(regs[instr.one.reg]) << std::endl;
							break;

						case VM::OneAddrPrintString:
							for(int i = regs[instr.one.reg]; mem[i] != '\0'; i++) {
								std::cout << (char)mem[i];
							}
							std::cout << std::endl;
							break;

						case VM::OneAddrCall:
							regs[VM::RegLR] = regs[VM::RegPC] + 4;
							regs[VM::RegPC] = regs[instr.one.reg] + instr.one.imm;
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