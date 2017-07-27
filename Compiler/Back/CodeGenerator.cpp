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

#undef LoadString

namespace Back {
	/*!
	 * \brief Generate code for an IR program
	 * \param irProgram IR program input
	 * \return Final code for the program
	 */
	void CodeGenerator::generate(IR::Program *irProgram, std::ostream &stream)
	{
		// Iterate through the procedures, generating each in turn
		for(std::unique_ptr<IR::Procedure> &irProcedure : irProgram->procedures()) {
			// Generate code for the procedure
			generateProcedure(*irProcedure, stream);
		}

		// Iterate through the data sections, generating each in turn
		for(IR::Data *irData : irProgram->data()) {
			// Generate code for the data
			generateData(irData, stream);
			stream << std::endl;
		}


	}

	/*!
	 * \brief Generate code for an IR procedure
	 * \param procedure Procedure to generate code for
	 * \param stream Stream to output assembly to
	 */
	void CodeGenerator::generateProcedure(IR::Procedure &procedure, std::ostream &stream)
	{
		std::map<IR::Symbol*, int> regMap;
		std::map<std::string, std::string> strings;
		std::string savedRegs;

		// Allocate registers for the procedure
		Util::Timer timer;
		timer.start();
		RegisterAllocator allocator;
		regMap = allocator.allocate(procedure);

		Util::log("opt.time") << "Register allocation (" << procedure.name() << "): " << timer.stop() << "ms" << std::endl;

		// Determine the set of registers that need to be saved/restored in the prologue/epilogue
		std::stringstream s;
		bool needComma = false;
		s << "{";
		for(auto &reg : regMap) {
			if(reg.second > 3) {
				if(needComma) {
					s << ", ";
				}
				s << "r" << reg.second;
				needComma = true;
			}
		}

		// If any calls are made in the procedure, LR must be saved as well
		for(IR::Entry *entry : procedure.entries()) {
			if(entry->type == IR::Entry::Type::Call || entry->type == IR::Entry::Type::CallIndirect) {
				if(needComma) {
					s << ", ";
				}
				s << "lr";
				break;
			}
		}
		s << "}";
		savedRegs = s.str();

		stream << "defproc " << procedure.name() << std::endl;

		// Iterate through each entry, and emit the appropriate code depending on its type
		for(IR::Entry *entry : procedure.entries()) {
			switch(entry->type) {
				case IR::Entry::Type::Move:
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

				case IR::Entry::Type::Add:
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

				case IR::Entry::Type::Mult:
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

				case IR::Entry::Type::Divide:
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

				case IR::Entry::Type::Modulo:
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

				case IR::Entry::Type::Equal:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    equ r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::Nequal:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    neq r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::LessThan:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    lt r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::LessThanE:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    lte r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::GreaterThan:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    gt r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::GreaterThanE:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    gte r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::Or:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    or r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::And:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    and r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << ", r" << regMap[threeAddr->rhs2] << std::endl;
						break;
					}

				case IR::Entry::Type::Label:
					{
						IR::EntryLabel *label = (IR::EntryLabel*)entry;
						stream << "  " << label->name << ":" << std::endl;
						break;
					}

				case IR::Entry::Type::Jump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						stream << "    jmp " << jump->target->name << std::endl;
						break;
					}

				case IR::Entry::Type::CJump:
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

				case IR::Entry::Type::Call:
					{
						IR::EntryCall *call = (IR::EntryCall*)entry;
						stream << "    call " << call->target << std::endl;
						break;
					}

				case IR::Entry::Type::CallIndirect:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    calli r" << regMap[threeAddr->rhs1] << std::endl;
						break;
					}

				case IR::Entry::Type::LoadRet:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->lhs] != 0) {
							stream << "    mov r" << regMap[threeAddr->lhs] << ", r0" << std::endl;
						}
						break;
					}

				case IR::Entry::Type::StoreRet:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(threeAddr->rhs1 && regMap[threeAddr->rhs1] != 0) {
							stream << "    mov r0, r" << regMap[threeAddr->rhs1] << std::endl;
						}
						break;
					}

				case IR::Entry::Type::LoadArg:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->lhs] != threeAddr->imm) {
							stream << "    mov r" << regMap[threeAddr->lhs] << ", r" << threeAddr->imm << std::endl;
						}
						break;
					}

				case IR::Entry::Type::StoreArg:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						if(regMap[threeAddr->rhs1] != threeAddr->imm) {
							stream << "    mov r" << threeAddr->imm << ", r" << regMap[threeAddr->rhs1] << std::endl;
						}
						break;
					}

				case IR::Entry::Type::LoadStack:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    ldr r" << regMap[threeAddr->lhs] << ", [sp, #" << threeAddr->imm << "]" << std::endl;
						break;
					}

				case IR::Entry::Type::StoreStack:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    str r" << regMap[threeAddr->rhs1] << ", [sp, #" << threeAddr->imm << "]" << std::endl;
						break;
					}

				case IR::Entry::Type::Prologue:
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

				case IR::Entry::Type::Epilogue:
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

				case IR::Entry::Type::New:
					{
						IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
						stream << "    new r" << regMap[threeAddr->lhs] << ", r" << regMap[threeAddr->rhs1] << std::endl;
						break;
					}

				case IR::Entry::Type::LoadMem:
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

				case IR::Entry::Type::StoreMem:
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

				case IR::Entry::Type::LoadString:
					{
						IR::EntryString *string = (IR::EntryString*)entry;
						std::stringstream s;
						s << "str" << strings.size();
						strings[s.str()] = string->string;
						stream << "    lea r" << regMap[string->lhs] << ", " << procedure.name() << "$$" << s.str() << std::endl;
						break;
					}

				case IR::Entry::Type::LoadAddress:
					{
						IR::EntryString *string = (IR::EntryString*)entry;
						stream << "    lea r" << regMap[string->lhs] << ", " << string->string << std::endl;
						break;
					}
			}
		}
		stream << std::endl;

		// Write out string constants
		for(auto &string : strings) {
			const std::string &name = string.first;
			const std::string &value = string.second;
			stream << "defdata " << procedure.name() << "$$" << name << std::endl;
			stream << "    string \"" << value << "\"" << std::endl;
			stream << std::endl;
		}
	}

	/*!
	 * \brief Generate code for an IR data section
	 * \param data Data to generate code for
	 * \param stream Stream to output assembly to
	 */
	void CodeGenerator::generateData(IR::Data *data, std::ostream &stream)
	{
		stream << "defdata " << data->name() << std::endl;

		// Iterate through each entry, and emit the appropriate code depending on its type
		for(IR::Entry *entry : data->entries()) {
			switch(entry->type) {
				case IR::Entry::Type::FunctionAddr:
					{
						IR::EntryCall *call = (IR::EntryCall*)entry;
						stream << "    addr " << call->target << std::endl;
						break;
					}
			}
		}
	}
}