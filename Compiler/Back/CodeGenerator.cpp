#include "Back/CodeGenerator.h"

#include "IR/Procedure.h"
#include "IR/Program.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "Back/RegisterAllocator.h"

#include "Transform/LiveRangeRenaming.h"

#include <map>

namespace Back {
	VM::Program CodeGenerator::generate(IR::Program *irProgram)
	{
		VM::Program vmProgram;

		std::map<IR::Procedure*, int> procedureMap;

		for(IR::Program::ProcedureList::iterator itProc = irProgram->procedures().begin(); itProc != irProgram->procedures().end(); itProc++) {
			IR::Procedure *irProcedure = *itProc;
			if(irProcedure->name() == "main") {
				vmProgram.start = (int)vmProgram.instructions.size();
			}

			procedureMap[irProcedure] = (int)vmProgram.instructions.size();
			generateProcedure(irProcedure, vmProgram.instructions, procedureMap);
		}

		return vmProgram;
	}

	void CodeGenerator::generateProcedure(IR::Procedure *procedure, std::vector<VM::Instruction> &instructions, const std::map<IR::Procedure*, int> &procedureMap)
	{
		std::map<IR::Symbol*, int> regMap;
		std::map<IR::EntryLabel*, int> labelMap;
		std::map<IR::Entry*, int> jumpMap;
		int reg = 1;

		RegisterAllocator allocator;

		Transform::LiveRangeRenaming::instance()->transform(procedure);

		regMap = allocator.allocate(procedure);

		instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegSP, VM::RegSP, -1));
		instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, VM::RegLR, 0));

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeMove:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], 0));
						break;
					}

				case IR::Entry::TypeLoadImm:
					{
						IR::EntryOneAddrImm *imm = (IR::EntryOneAddrImm*)entry;
						instructions.push_back(VM::Instruction::makeOneAddr(VM::OneAddrLoadImm, regMap[imm->lhs], imm->imm));
						break;
					}

				case IR::Entry::TypeAdd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrAdd, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypeAddImm:
					{
						IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[twoAddrImm->lhs], regMap[twoAddrImm->rhs], twoAddrImm->imm));
						break;
					}

				case IR::Entry::TypeMult:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrMult, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypePrint:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeOneAddr(VM::OneAddrPrint, regMap[threeAddr->rhs1], 0));
						break;
					}

				case IR::Entry::TypeEqual:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrEqual, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypeNequal:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrNEqual, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
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
						VM::Instruction instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddCond, 0, 0, 0, 0);
						jumpMap[entry] = (int)instructions.size();
						instructions.push_back(instr);
						instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, 0, 0, 0);
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypeCall:
					{
						IR::EntryCall *call = (IR::EntryCall*)entry;
						int offset = procedureMap.find(call->target)->second;
						instructions.push_back(VM::Instruction::makeOneAddr(VM::OneAddrCall, VM::RegPC, offset - (int)instructions.size()));
						break;
					}

				case IR::Entry::TypeLoadRet:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->lhs] != 0) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[threeAddr->lhs], 0, 0));
						}
						break;
					}

				case IR::Entry::TypeStoreRet:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->rhs1] != 0) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, 0, regMap[threeAddr->rhs1], 0));
						}
						break;
					}

				case IR::Entry::TypeLoadArg:
					{
						IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;
						if(regMap[twoAddrImm->lhs] != twoAddrImm->imm) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[twoAddrImm->lhs], twoAddrImm->imm, 0));
						}
						break;
					}

				case IR::Entry::TypeStoreArg:
					{
						IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;
						if(regMap[twoAddrImm->rhs] != twoAddrImm->imm) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, twoAddrImm->imm, regMap[twoAddrImm->rhs], 0));
						}
						break;
					}
			}
		}

		instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegSP, VM::RegSP, 1));
		instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, VM::RegPC, VM::RegSP, -1));

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
						instructions[idx] = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddCond, VM::RegPC, regMap[cjump->pred], VM::RegPC, target - idx);
						target = labelMap[cjump->falseTarget];
						instructions[idx+1] = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegPC, target - idx - 1);
						break;
					}
			}
		}
	}
}