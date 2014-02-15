#include "Back/CodeGenerator.h"

#include "IR/Procedure.h"
#include "IR/Program.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "Back/RegisterAllocator.h"

#include "Util/Timer.h"

#include <map>

namespace Back {
	/*!
	 * \brief Generate code for an IR program
	 * \param irProgram IR program input
	 * \return Final code for the program
	 */
	VM::Program CodeGenerator::generate(IR::Program *irProgram)
	{
		VM::Program vmProgram;

		std::map<IR::Procedure*, int> procedureMap;

		// Iterate through the procedures, generating each in turn
		for(IR::ProcedureList::iterator itProc = irProgram->procedures().begin(); itProc != irProgram->procedures().end(); itProc++) {
			IR::Procedure *irProcedure = *itProc;

			// If this procedure is main(), save its location as the program start point
			if(irProcedure->name() == "main") {
				vmProgram.start = (int)vmProgram.instructions.size() * 4;
			}

			// Save the procedure's start location, so that call instructions can be directed
			// to the correct point
			procedureMap[irProcedure] = (int)vmProgram.instructions.size() * 4;

			// Generate code for the procedure
			generateProcedure(irProcedure, vmProgram.instructions, procedureMap);
		}

		return vmProgram;
	}

	/*!
	 * \brief Generate code for an IR procedure
	 * \param procedure Procedure to generate code for
	 * \param instructions Instruction stream to write to
	 * \param procedureMap Map of starting locations for procedures
	 */
	void CodeGenerator::generateProcedure(IR::Procedure *procedure, std::vector<unsigned char> &data, const std::map<IR::Procedure*, int> &procedureMap)
	{
		std::map<IR::Symbol*, int> regMap;
		std::map<IR::EntryLabel*, int> labelMap;
		std::map<IR::Entry*, int> jumpMap;
		std::vector<VM::Instruction> instructions;
		int reg = 1;

		// Allocate registers for the procedure
		Util::Timer timer;
		timer.start();
		RegisterAllocator allocator;
		regMap = allocator.allocate(procedure);

		std::cout << "Register allocation (" << procedure->name() << "): " << timer.stop() << "ms" << std::endl;

		// Determine the set of registers that need to be saved/restored in the prologue/epilogue
		unsigned long savedRegs = 0;
		for(std::map<IR::Symbol*, int>::iterator regIt = regMap.begin(); regIt != regMap.end(); regIt++) {
			if(regIt->second > 3) {
				savedRegs |= (1 << regIt->second);
			}
		}

		// If any calls are made in the procedure, LR must be saved as well
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			if(entry->type == IR::Entry::TypeCall) {
				savedRegs |= (1 << VM::RegLR);
			}
		}

		// Iterate through each entry, and emit the appropriate code depending on its type
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeMove:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(threeAddr->rhs1) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], 0));
						} else {
							instructions.push_back(VM::Instruction::makeOneAddr(VM::OneAddrLoadImm, regMap[threeAddr->lhs], threeAddr->imm));
						}
						break;
					}

				case IR::Entry::TypeAdd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(threeAddr->rhs2) {
							instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrAdd, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						} else {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], threeAddr->imm));
						}
						break;
					}

				case IR::Entry::TypeMult:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(threeAddr->rhs2) {
							instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrMult, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						} else {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrMultImm, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], threeAddr->imm));
						}
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

				case IR::Entry::TypeLessThan:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrLessThan, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypeLessThanE:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrLessThanE, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypeGreaterThan:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrGreaterThan, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypeGreaterThanE:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrGreaterThanE, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypeOr:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrOr, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
						break;
					}

				case IR::Entry::TypeAnd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrAnd, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], 0));
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
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->lhs] != threeAddr->imm) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, regMap[threeAddr->lhs], threeAddr->imm, 0));
						}
						break;
					}

				case IR::Entry::TypeStoreArg:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->rhs1] != threeAddr->imm) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, threeAddr->imm, regMap[threeAddr->rhs1], 0));
						}
						break;
					}

				case IR::Entry::TypeLoadStack:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, regMap[threeAddr->lhs], VM::RegSP, threeAddr->imm));
						break;
					}

				case IR::Entry::TypeStoreStack:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, VM::RegSP, regMap[threeAddr->rhs1], threeAddr->imm));
						break;
					}

				case IR::Entry::TypePrologue:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						// Save all required registers for this function
						if(savedRegs != 0) {
							instructions.push_back(VM::Instruction::makeMultReg(VM::MultRegStore, savedRegs));
						}

						// Make space on the stack frame for any necessary spilled values
						if(threeAddr->imm > 0) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegSP, VM::RegSP, -threeAddr->imm));
						}
						break;
					}

				case IR::Entry::TypeEpilogue:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;

						// Advance the stack pointer past the spilled value range
						if(threeAddr->imm > 0) {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegSP, VM::RegSP, threeAddr->imm));
						}

						// Reload all required registers
						if(savedRegs != 0) {
							instructions.push_back(VM::Instruction::makeMultReg(VM::MultRegLoad, savedRegs));
						}

						// Jump back to the return location
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegLR, 0));
						break;
					}

				case IR::Entry::TypeNew:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrNew, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], 0));
						break;
					}

				case IR::Entry::TypeLoadMem:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(threeAddr->rhs2) {
							instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrLoad, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], threeAddr->imm));
						} else {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrLoad, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], threeAddr->imm));
						}
						break;
					}

				case IR::Entry::TypeStoreMem:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(threeAddr->rhs2) {
							instructions.push_back(VM::Instruction::makeThreeAddr(VM::ThreeAddrStore, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], regMap[threeAddr->rhs2], threeAddr->imm));
						} else {
							instructions.push_back(VM::Instruction::makeTwoAddr(VM::TwoAddrStore, regMap[threeAddr->lhs], regMap[threeAddr->rhs1], threeAddr->imm));
						}
						break;
					}
			}
		}

		// Now that all locations have been placed, loop back through the entries and fill in jump target locations
		for(std::map<IR::Entry*, int>::iterator it = jumpMap.begin(); it != jumpMap.end(); it++) {
			IR::Entry *entry = it->first;
			int idx = it->second;

			switch(entry->type) {
				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						int target = labelMap[jump->target];
						instructions[idx] = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegPC, 4 * (target - idx));
						break;
					}

				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *cjump = (IR::EntryCJump*)entry;
						int target = labelMap[cjump->trueTarget];
						instructions[idx] = VM::Instruction::makeThreeAddr(VM::ThreeAddrAddCond, VM::RegPC, regMap[cjump->pred], VM::RegPC, 4 * (target - idx));
						target = labelMap[cjump->falseTarget];
						instructions[idx+1] = VM::Instruction::makeTwoAddr(VM::TwoAddrAddImm, VM::RegPC, VM::RegPC, 4 * (target - idx - 1));
						break;
					}
			}
		}

		data.resize(instructions.size() * 4);
		std::memcpy(&data[0], &instructions[0], instructions.size() * 4);
	}
}