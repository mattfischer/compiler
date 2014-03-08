#include "VM/Interp.h"

#include "VM/Heap.h"

#include <sstream>

namespace VM {
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
	void Interp::run(const Program *program)
	{
		int regs[16];
		int curPC;
		unsigned char mem[1024];
		Heap heap(mem, 1024, (int)program->instructions.size());

		if(program->symbols.find("main") == program->symbols.end()) {
			std::cout << "Error: Undefined reference to main" << std::endl;
			return;
		}

		if(program->imports.size() > 0) {
			std::cout << "Error: Program has unresolved imports" << std::endl;
			return;
		}

		// Initialize all registers to 0
		memset(regs, 0, 16 * sizeof(int));

		memcpy(mem, &program->instructions[0], program->instructions.size());

		// Set SP to the top of the stack
		regs[VM::RegSP] = sizeof(mem);

		// Set LR to beyond the end of the program, so program exit can be detected
		regs[VM::RegLR] = 0xffffffff;

		// Set PC to the program entry point
		regs[VM::RegPC] = program->symbols.find("main")->second;

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

						case VM::TwoAddrDivImm:
							regs[instr.two.regLhs] = regs[instr.two.regRhs] / instr.two.imm;
							break;

						case VM::TwoAddrModImm:
							regs[instr.two.regLhs] = regs[instr.two.regRhs] % instr.two.imm;
							break;

						case VM::TwoAddrLoad:
							regs[instr.two.regLhs] = *((unsigned long*)(&mem[regs[instr.two.regRhs] + instr.two.imm]));
							break;

						case VM::TwoAddrStore:
							*((unsigned long*)(&mem[regs[instr.two.regRhs] + instr.two.imm])) = regs[instr.two.regLhs];
							break;

						case VM::TwoAddrNew:
							regs[instr.two.regLhs] = heap.allocate(regs[instr.two.regRhs]);
							break;

						case VM::TwoAddrStringInt:
							regs[instr.two.regLhs] = strInt(mem, heap, regs[instr.two.regRhs]);
							break;

						case VM::TwoAddrLoadByte:
							regs[instr.two.regLhs] = mem[regs[instr.two.regRhs] + instr.two.imm];
							break;

						case VM::TwoAddrStoreByte:
							mem[regs[instr.two.regRhs] + instr.two.imm] = regs[instr.two.regLhs] & 0xff;
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

						case VM::ThreeAddrDiv:
							regs[instr.three.regLhs] = regs[instr.three.regRhs1] / regs[instr.three.regRhs2];
							break;

						case VM::ThreeAddrMod:
							regs[instr.three.regLhs] = regs[instr.three.regRhs1] % regs[instr.three.regRhs2];
							break;

						case VM::ThreeAddrAddCond:
							if(regs[instr.three.regRhs1]) {
								regs[instr.three.regLhs] = regs[instr.three.regRhs2] + instr.three.imm;
							}
							break;

						case VM::ThreeAddrAddNCond:
							if(!regs[instr.three.regRhs1]) {
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
							regs[instr.three.regLhs] = *(unsigned long*)(mem + regs[instr.three.regRhs1] + (regs[instr.three.regRhs2] << instr.three.imm));
							break;

						case VM::ThreeAddrStore:
							*(unsigned long*)(mem + regs[instr.three.regRhs1] + (regs[instr.three.regRhs2] << instr.three.imm)) = regs[instr.three.regLhs];
							break;

						case VM::ThreeAddrLoadByte:
							regs[instr.three.regLhs] = mem[regs[instr.three.regRhs1] + (regs[instr.three.regRhs2] << instr.three.imm)];
							break;

						case VM::ThreeAddrStoreByte:
							mem[regs[instr.three.regRhs1] + (regs[instr.three.regRhs2] << instr.three.imm)] = regs[instr.three.regLhs] & 0xff;
							break;
					}
					break;

				case VM::InstrMultReg:
					switch(instr.mult.type) {
						case VM::MultRegLoad:
							for(int i=0; i<16; i++) {
								if(instr.mult.regs & (1 << i)) {
									regs[i] = *(int*)(mem + regs[instr.mult.lhs]);
									regs[instr.mult.lhs] += sizeof(int);
								}
							}
							break;

						case VM::MultRegStore:
							for(int i=15; i>=0; i--) {
								if(instr.mult.regs & (1 << i)) {
									regs[instr.mult.lhs] -= sizeof(int);
									*(int*)(mem + regs[instr.mult.lhs]) = regs[i];
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