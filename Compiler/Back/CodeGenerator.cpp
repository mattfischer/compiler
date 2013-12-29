#include "Back/CodeGenerator.h"

#include "IR/Procedure.h"
#include "IR/Program.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include <map>

namespace Back {
	VM::Program CodeGenerator::generate(IR::Program *irProgram)
	{
		VM::Program vmProgram;

		for(IR::Program::ProcedureList::iterator itProc = irProgram->procedures().begin(); itProc != irProgram->procedures().end(); itProc++) {
			IR::Procedure *irProcedure = *itProc;
			if(irProcedure->name() == "main") {
				vmProgram.start = (int)vmProgram.instructions.size();
			}
			generateProcedure(irProcedure, vmProgram.instructions);
		}

		return vmProgram;
	}

	void CodeGenerator::generateProcedure(IR::Procedure *procedure, std::vector<VM::Instruction> &instructions)
	{
		std::map<IR::Symbol*, int> regMap;
		std::map<IR::EntryLabel*, int> labelMap;
		std::map<IR::Entry*, int> jumpMap;
		int reg = 0;

		for(IR::Procedure::SymbolList::iterator itSymbol = procedure->symbols().begin(); itSymbol != procedure->symbols().end(); itSymbol++) {
			IR::Symbol *symbol = *itSymbol;
			regMap[symbol] = reg++;
		}

		instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegSP, VM::RegSP, -(int)regMap.size()));

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeMove:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[threeAddr->rhs1]));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, 0, regMap[threeAddr->lhs]));
						break;
					}

				case IR::Entry::TypeLoadImm:
					{
						IR::EntryOneAddrImm *imm = (IR::EntryOneAddrImm*)entry;
						instructions.push_back(VM::Instruction::makeOneAddr(VM::OneAddrLoadImm, 0, imm->imm));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, 0, regMap[imm->lhs]));
						break;
					}

				case IR::Entry::TypeAdd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[threeAddr->rhs1]));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 1, VM::RegSP, regMap[threeAddr->rhs2]));
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrAdd, 0, 0, 1, 0));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, 0, regMap[threeAddr->lhs]));
						break;
					}

				case IR::Entry::TypeAddImm:
					{
						IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[twoAddrImm->rhs]));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, 0, 0, twoAddrImm->imm));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, 0, regMap[twoAddrImm->lhs]));
						break;
					}

				case IR::Entry::TypeMult:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[threeAddr->rhs1]));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 1, VM::RegSP, regMap[threeAddr->rhs2]));
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrMult, 0, 0, 1, 0));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, 0, regMap[threeAddr->lhs]));
						break;
					}

				case IR::Entry::TypePrint:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[threeAddr->rhs1]));
						instructions.push_back(VM::Instruction::makeOneAddr(VM::OneAddrPrint, 0, 0));
						break;
					}

				case IR::Entry::TypeEqual:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[threeAddr->rhs1]));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 1, VM::RegSP, regMap[threeAddr->rhs2]));
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrEqual, 0, 0, 1, 0));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, 0, regMap[threeAddr->lhs]));
						break;
					}

				case IR::Entry::TypeNequal:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[threeAddr->rhs1]));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 1, VM::RegSP, regMap[threeAddr->rhs2]));
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrNEqual, 0, 0, 1, 0));
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, 0, regMap[threeAddr->lhs]));

						break;
					}

				case IR::Entry::TypeLabel:
					{
						IR::EntryLabel *label = (IR::EntryLabel*)entry;
						labelMap[label] = (int)instructions.size();
						break;
					}

				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						VM::Instruction instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, 0, 0, 0);
						jumpMap[entry] = (int)instructions.size();
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *cjump = (IR::EntryCJump*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, 0, VM::RegSP, regMap[cjump->pred]));
						VM::Instruction instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddCond, 0, 0, 0, 0);
						jumpMap[entry] = (int)instructions.size();
						instructions.push_back(instr);
						instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, 0, 0, 0);
						instructions.push_back(instr);
						break;
					}
			}
		}

		instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegSP, VM::RegSP, (int)regMap.size()));

		for(std::map<IR::Entry*, int>::iterator it = jumpMap.begin(); it != jumpMap.end(); it++) {
			IR::Entry *entry = it->first;
			int idx = it->second;

			switch(entry->type) {
				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						int target = labelMap[jump->target];
						instructions[idx] = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegPC, target - idx);
						break;
					}

				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *cjump = (IR::EntryCJump*)entry;
						int target = labelMap[cjump->trueTarget];
						instructions[idx] = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddCond, VM::RegPC, 0, VM::RegPC, target - idx);
						target = labelMap[cjump->falseTarget];
						instructions[idx+1] = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegPC, target - idx - 1);
						break;
					}
			}
		}
	}
}