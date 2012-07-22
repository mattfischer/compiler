#include "Back/CodeGenerator.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include <map>

namespace Back {
	std::vector<VM::Instruction> CodeGenerator::generate(IR::Procedure *procedure)
	{
		std::vector<VM::Instruction> instructions;
		std::map<IR::Symbol*, int> regMap;
		std::map<IR::EntryLabel*, int> labelMap;
		std::map<IR::Entry*, int> jumpMap;
		int reg = 0;

		for(IR::Procedure::SymbolList::iterator itSymbol = procedure->symbols().begin(); itSymbol != procedure->symbols().end(); itSymbol++) {
			IR::Symbol *symbol = *itSymbol;
			regMap[symbol] = reg++;
		}

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeLoad:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						VM::Instruction instr = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], 0);
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypeLoadImm:
					{
						IR::EntryImm *imm = (IR::EntryImm*)entry;
						VM::Instruction instr = VM::Instruction::makeOneAddr(VM::OneAddrLoadImm, regMap[imm->lhs], imm->rhs);
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypeAdd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						VM::Instruction instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrAdd, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0);
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypeMult:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						VM::Instruction instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrMult, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0);
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypePrint:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						VM::Instruction instr = VM::Instruction::makeOneAddr(VM::OneAddrPrint, regMap[threeAddr->rhs1], 0);
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypeEqual:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						VM::Instruction instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrEqual, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0);
						instructions.push_back(instr);
						break;
					}

				case IR::Entry::TypeNequal:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						VM::Instruction instr = VM::Instruction::makeThreeAddr(VM::ThreeAddrNEqual, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0);
						instructions.push_back(instr);
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
			}
		}

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

		return instructions;
	}
}