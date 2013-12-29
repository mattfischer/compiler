#include "VM/Interp.h"

namespace VM {
	void Interp::run(std::vector<VM::Instruction> &instructions)
	{
		int regs[16];
		int curPC;
		int mem[1024];

		memset(regs, 0, 16 * sizeof(int));
		regs[VM::RegSP] = sizeof(mem) / sizeof(int) - 1;

		while(regs[VM::RegPC] < (int)instructions.size()) {
			curPC = regs[VM::RegPC];
			Instruction instr = instructions[regs[VM::RegPC]];

			switch(instr.type) {
				case VM::InstrOneAddr:
					switch(instr.u.one.type) {
						case VM::OneAddrLoadImm:
							regs[instr.u.one.reg] = instr.u.one.imm;
							break;

						case VM::OneAddrPrint:
							printf("%i\n", regs[instr.u.one.reg]);
							break;
					}
					break;

				case VM::InstrTwoAddr:
					switch(instr.u.two.type) {
						case VM::TwoAddrAddImm:
							regs[instr.u.two.regDst] = regs[instr.u.two.regSrc] + instr.u.two.imm;
							break;

						case VM::TwoAddrLoad:
							regs[instr.u.two.regDst] = mem[regs[instr.u.two.regSrc] + instr.u.two.imm];
							break;

						case::VM::TwoAddrStore:
							mem[regs[instr.u.two.regDst] + instr.u.two.imm] = regs[instr.u.two.regSrc];
							break;
					}
					break;

				case VM::InstrThreeAddr:
					switch(instr.u.three.type) {
						case VM::ThreeAddrAdd:
							regs[instr.u.three.regDst] = regs[instr.u.three.regSrc1] + regs[instr.u.three.regSrc2];
							break;

						case VM::ThreeAddrMult:
							regs[instr.u.three.regDst] = regs[instr.u.three.regSrc1] * regs[instr.u.three.regSrc2];
							break;

						case VM::ThreeAddrAddCond:
							if(regs[instr.u.three.regSrc1]) {
								regs[instr.u.three.regDst] = regs[instr.u.three.regSrc2] + instr.u.three.imm;
							}
							break;

						case VM::ThreeAddrEqual:
							regs[instr.u.three.regDst] = (regs[instr.u.three.regSrc1] == regs[instr.u.three.regSrc2]);
							break;

						case VM::ThreeAddrNEqual:
							regs[instr.u.three.regDst] = (regs[instr.u.three.regSrc1] != regs[instr.u.three.regSrc2]);
							break;
					}
					break;
			}

			if(regs[VM::RegPC] == curPC) {
				regs[VM::RegPC]++;
			}
		}
	}
}