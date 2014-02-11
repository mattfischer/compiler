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
		Heap heap(mem, 1024, 0);

		// Initialize all registers to 0
		memset(regs, 0, 16 * sizeof(int));

		// Set SP to the top of the stack
		regs[VM::RegSP] = sizeof(mem) - 1;

		// Set LR to beyond the end of the program, so program exit can be detected
		regs[VM::RegLR] = (int)program.instructions.size();

		// Set PC to the program entry point
		regs[VM::RegPC] = program.start;

		// Loop until PC is set beyond the end of the program
		while(regs[VM::RegPC] < (int)program.instructions.size()) {
			curPC = regs[VM::RegPC];
			Instruction instr = program.instructions[regs[VM::RegPC]];

			// Examine the instruction, and interpret it accordingly
			switch(instr.type) {
				case VM::InstrOneAddr:
					switch(instr.u.one.type) {
						case VM::OneAddrLoadImm:
							regs[instr.u.one.reg] = instr.u.one.imm;
							break;

						case VM::OneAddrPrint:
							std::cout << int(regs[instr.u.one.reg]) << std::endl;
							break;

						case VM::OneAddrCall:
							regs[VM::RegLR] = regs[VM::RegPC] + 1;
							regs[VM::RegPC] = regs[instr.u.one.reg] + instr.u.one.imm;
							break;
					}
					break;

				case VM::InstrTwoAddr:
					switch(instr.u.two.type) {
						case VM::TwoAddrAddImm:
							regs[instr.u.two.regLhs] = regs[instr.u.two.regRhs] + instr.u.two.imm;
							break;

						case VM::TwoAddrMultImm:
							regs[instr.u.two.regLhs] = regs[instr.u.two.regRhs] * instr.u.two.imm;
							break;

						case VM::TwoAddrLoad:
							regs[instr.u.two.regLhs] = mem[regs[instr.u.two.regRhs] + instr.u.two.imm];
							break;

						case VM::TwoAddrStore:
							mem[regs[instr.u.two.regRhs] + instr.u.two.imm] = regs[instr.u.two.regLhs];
							break;

						case VM::TwoAddrNew:
							regs[instr.u.two.regLhs] = heap.allocate(regs[instr.u.two.regRhs]);
							break;
					}
					break;

				case VM::InstrThreeAddr:
					switch(instr.u.three.type) {
						case VM::ThreeAddrAdd:
							regs[instr.u.three.regLhs] = regs[instr.u.three.regRhs1] + regs[instr.u.three.regRhs2];
							break;

						case VM::ThreeAddrSub:
							regs[instr.u.three.regLhs] = regs[instr.u.three.regRhs1] - regs[instr.u.three.regRhs2];
							break;

						case VM::ThreeAddrMult:
							regs[instr.u.three.regLhs] = regs[instr.u.three.regRhs1] * regs[instr.u.three.regRhs2];
							break;

						case VM::ThreeAddrAddCond:
							if(regs[instr.u.three.regRhs1]) {
								regs[instr.u.three.regLhs] = regs[instr.u.three.regRhs2] + instr.u.three.imm;
							}
							break;

						case VM::ThreeAddrEqual:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] == regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrNEqual:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] != regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrLessThan:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] < regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrLessThanE:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] <= regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrGreaterThan:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] > regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrGreaterThanE:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] >= regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrOr:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] || regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrAnd:
							regs[instr.u.three.regLhs] = (regs[instr.u.three.regRhs1] && regs[instr.u.three.regRhs2]);
							break;

						case VM::ThreeAddrLoad:
							regs[instr.u.three.regLhs] = *(int*)(mem + regs[instr.u.three.regRhs1] + (regs[instr.u.three.regRhs2] << instr.u.three.imm));
							break;

						case VM::ThreeAddrStore:
							*(int*)(mem + regs[instr.u.three.regRhs1] + (regs[instr.u.three.regRhs2] << instr.u.three.imm)) = regs[instr.u.three.regLhs];
							break;
					}
					break;

				case VM::InstrMultReg:
					switch(instr.u.mult.type) {
						case VM::MultRegLoad:
							for(int i=0; i<16; i++) {
								if(instr.u.mult.regs & (1 << i)) {
									regs[i] = *(int*)(mem + regs[VM::RegSP]);
									regs[VM::RegSP] += sizeof(int);
								}
							}
							break;

						case VM::MultRegStore:
							for(int i=15; i>=0; i--) {
								if(instr.u.mult.regs & (1 << i)) {
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
				regs[VM::RegPC]++;
			}
		}
	}
}