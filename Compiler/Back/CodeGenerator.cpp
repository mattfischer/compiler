#include "Back/CodeGenerator.h"

#include "IR/Procedure.h"
#include "IR/Program.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "VM/Instruction.h"

#include "Back/RegisterAllocator.h"

#include "Util/Timer.h"
#include "Util/Log.h"

#include <map>
#include <sstream>

namespace Back {
	/*!
	 * \brief Generate code for an IR program
	 * \param irProgram IR program input
	 * \return Final code for the program
	 */
	void CodeGenerator::generate(IR::Program *irProgram, std::ostream &stream)
	{
		// Iterate through the procedures, generating each in turn
		for(IR::ProcedureList::iterator itProc = irProgram->procedures().begin(); itProc != irProgram->procedures().end(); itProc++) {
			IR::Procedure *irProcedure = *itProc;

			// Generate code for the procedure
			generateProcedure(irProcedure, stream);
			stream << std::endl;
		}
	}

	/*!
	 * \brief Substitute IR entries which are implemented by library calls
	 * \param procedure Procedure to modify
	 */
	static void substituteLibraryCalls(IR::Procedure *procedure)
	{
		for(IR::EntryList::iterator it = procedure->entries().begin(); it != procedure->entries().end(); it++) {
			IR::Entry *entry = *it;
			IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;

			switch(entry->type) {
				case IR::Entry::TypeStringConcat:
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, threeAddr->rhs1, 0, 0));
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, threeAddr->rhs2, 0, 1));
					procedure->entries().insert(entry, new IR::EntryCall("__string_concat"));
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, threeAddr->lhs));
					procedure->entries().erase(it);
					it--;
					delete entry;
					break;

				case IR::Entry::TypeStringBool:
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, threeAddr->rhs1, 0, 0));
					procedure->entries().insert(entry, new IR::EntryCall("__string_bool"));
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, threeAddr->lhs));
					procedure->entries().erase(it);
					it--;
					delete entry;
					break;

				case IR::Entry::TypeStringChar:
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, threeAddr->rhs1, 0, 0));
					procedure->entries().insert(entry, new IR::EntryCall("__string_char"));
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, threeAddr->lhs));
					procedure->entries().erase(it);
					it--;
					delete entry;
					break;

				case IR::Entry::TypeStringInt:
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeStoreArg, 0, threeAddr->rhs1, 0, 0));
					procedure->entries().insert(entry, new IR::EntryCall("__string_int"));
					procedure->entries().insert(entry, new IR::EntryThreeAddr(IR::Entry::TypeLoadRet, threeAddr->lhs));
					procedure->entries().erase(it);
					it--;
					delete entry;
					break;			
			}
		}
	}

	/*!
	 * \brief Generate code for an IR procedure
	 * \param procedure Procedure to generate code for
	 * \param stream Stream to output assembly to
	 */
	void CodeGenerator::generateProcedure(IR::Procedure *procedure, std::ostream &stream)
	{
		std::map<IR::Symbol*, int> regMap;
		std::map<std::string, std::string> strings;
		std::string savedRegs;

		// Insert library calls
		substituteLibraryCalls(procedure);

		// Allocate registers for the procedure
		Util::Timer timer;
		timer.start();
		RegisterAllocator allocator;
		regMap = allocator.allocate(procedure);

		Util::log("opt.time") << "Register allocation (" << procedure->name() << "): " << timer.stop() << "ms" << std::endl;

		// Determine the set of registers that need to be saved/restored in the prologue/epilogue
		std::stringstream s;
		bool needComma = false;
		s << "{";
		for(std::map<IR::Symbol*, int>::iterator regIt = regMap.begin(); regIt != regMap.end(); regIt++) {
			if(regIt->second > 3) {
				if(needComma) {
					s << ", ";
				}
				s << "r" << regIt->second;
				needComma = true;
			}
		}

		// If any calls are made in the procedure, LR must be saved as well
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			if(entry->type == IR::Entry::TypeCall) {
				if(needComma) {
					s << ", ";
				}
				s << "lr";
			}
		}
		s << "}";
		savedRegs = s.str();

		stream << "defproc " << procedure->name() << std::endl;

		// Iterate through each entry, and emit the appropriate code depending on its type
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeMove:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    mov r" << regMap[threeAddr->lhs] << ", ";
						if(threeAddr->rhs1) {
							stream << "r" << regMap[threeAddr->rhs1];
						} else {
							stream << "#" << threeAddr->imm;
						}
						stream << std::endl;
						break;
					}

				case IR::Entry::TypeAdd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    add r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", ";
						if(threeAddr->rhs2) {
							stream << "r" << regMap[threeAddr->rhs2];
						} else {
							stream << "#" << threeAddr->imm;
						}
						stream << std::endl;
						break;
					}

				case IR::Entry::TypeMult:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    mult r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", ";
						if(threeAddr->rhs2) {
							stream << "r" << regMap[threeAddr->rhs2];
						} else {
							stream << "#" << threeAddr->imm;
						}
						stream << std::endl;
						break;
					}

				case IR::Entry::TypeDivide:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    div r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", ";
						if(threeAddr->rhs2) {
							stream << "r" << regMap[threeAddr->rhs2];
						} else {
							stream << "#" << threeAddr->imm;
						}
						stream << std::endl;
						break;
					}

				case IR::Entry::TypeModulo:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    mod r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", ";
						if(threeAddr->rhs2) {
							stream << "r" << regMap[threeAddr->rhs2];
						} else {
							stream << "#" << threeAddr->imm;
						}
						stream << std::endl;
						break;
					}

				case IR::Entry::TypePrint:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    print r" << regMap[threeAddr->rhs1] << std::endl;
						break;
					}

				case IR::Entry::TypeEqual:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    equ r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeNequal:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    neq r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeLessThan:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    lt r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeLessThanE:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    lte r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeGreaterThan:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    gt r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeGreaterThanE:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    gte r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeOr:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    or r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeAnd:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    and r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::TypeLabel:
					{
						IR::EntryLabel *label = (IR::EntryLabel*)entry;
						stream << "  " << label->name << ":" << std::endl;
						break;
					}

				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						stream << "    jmp " << jump->target->name << std::endl;
						break;
					}

				case IR::Entry::TypeCJump:
					{
						IR::EntryCJump *cjump = (IR::EntryCJump*)entry;
						if(cjump->next == cjump->trueTarget) {
							stream << "    ncjmp r" << regMap[cjump->pred] << ", " << cjump->falseTarget->name << std::endl;
						} else if(cjump->next == cjump->falseTarget) {
							stream << "    cjmp r" << regMap[cjump->pred] << ", " << cjump->trueTarget->name << std::endl;
						} else {
							stream << "    cjmp r" << regMap[cjump->pred] << ", " << cjump->trueTarget->name << std::endl;
							stream << "    jmp " << cjump->falseTarget->name << std::endl;
						}
						break;
					}

				case IR::Entry::TypeCall:
					{
						IR::EntryCall *call = (IR::EntryCall*)entry;
						stream << "    call " << call->target << std::endl;
						break;
					}

				case IR::Entry::TypeLoadRet:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->lhs] != 0) {
							stream << "    mov r" << regMap[threeAddr->lhs] << ", r0" << std::endl;
						}
						break;
					}

				case IR::Entry::TypeStoreRet:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(threeAddr->rhs1 && regMap[threeAddr->rhs1] != 0) {
							stream << "    mov r0, r" << regMap[threeAddr->rhs1] << std::endl;
						}
						break;
					}

				case IR::Entry::TypeLoadArg:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->lhs] != threeAddr->imm) {
							stream << "    mov r" << regMap[threeAddr->lhs] << ", r" << threeAddr->imm << std::endl;
						}
						break;
					}

				case IR::Entry::TypeStoreArg:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->rhs1] != threeAddr->imm) {
							stream << "    mov r" << threeAddr->imm << ", r" << regMap[threeAddr->rhs1] << std::endl;
						}
						break;
					}

				case IR::Entry::TypeLoadStack:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    ldr r" << regMap[threeAddr->lhs] << ", [sp, #" << threeAddr->imm << "]" << std::endl;
						break;
					}

				case IR::Entry::TypeStoreStack:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    str r" << regMap[threeAddr->rhs1] << ", [sp, #" << threeAddr->imm << "]" << std::endl;
						break;
					}

				case IR::Entry::TypePrologue:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						// Save all required registers for this function
						if(savedRegs != "{}") {
							stream << "    stm sp, " << savedRegs << std::endl;
						}

						// Make space on the stack frame for any necessary spilled values
						if(threeAddr->imm > 0) {
							stream << "    sub sp, sp, #" << threeAddr->imm << std::endl;
						}
						break;
					}

				case IR::Entry::TypeEpilogue:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;

						// Advance the stack pointer past the spilled value range
						if(threeAddr->imm > 0) {
							stream << "    add sp, sp, #" << threeAddr->imm << std::endl;
						}

						// Reload all required registers
						if(savedRegs != "{}") {
							stream << "    ldm sp, " << savedRegs << std::endl;
						}

						// Jump back to the return location
						stream << "    mov pc, lr" << std::endl;
						break;
					}

				case IR::Entry::TypeNew:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    new r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << std::endl;
						break;
					}

				case IR::Entry::TypeLoadMem:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						std::string opcode;
						if(threeAddr->lhs->size == 1) {
							opcode = "ldb";
						} else {
							opcode = "ldr";
						}

						stream << "    " << opcode << " r" << regMap[threeAddr->lhs] << ", [r" << regMap[threeAddr->rhs1];
						if(threeAddr->rhs2) {
							stream << ", r" << regMap[threeAddr->rhs2];
						}

						if(threeAddr->imm != 0) {
							stream << ", #" << threeAddr->imm;
						}

						stream << "]" << std::endl;
						break;
					}

				case IR::Entry::TypeStoreMem:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						std::string opcode;
						if(threeAddr->lhs->size == 1) {
							opcode = "stb";
						} else {
							opcode = "str";
						}
						stream << "    " << opcode << " r" << regMap[threeAddr->lhs] << ", [r" << regMap[threeAddr->rhs1];
						if(threeAddr->rhs2) {
							stream << ", r" << regMap[threeAddr->rhs2];
						}

						if(threeAddr->imm != 0) {
							stream << ", #" << threeAddr->imm;
						}

						stream << "]" << std::endl;
						break;
					}

				case IR::Entry::TypeLoadString:
					{
						IR::EntryString *string = (IR::EntryString*)entry;
						std::stringstream s;
						s << "str" << strings.size();
						strings[s.str()] = string->string;
						stream << "    lea r" << regMap[string->lhs] << ", " << s.str() << std::endl;
						break;
					}

				case IR::Entry::TypeStringInt:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    strint r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << std::endl;
						break;
					}

			}
		}

		// Write out string constants
		for(std::map<std::string, std::string>::iterator it = strings.begin(); it != strings.end(); it++) {
			const std::string &name = it->first;
			const std::string &value = it->second;
			stream << "  string " << name << ", \"" << value << "\"" << std::endl;
		}
	}
}