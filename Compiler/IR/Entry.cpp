#include "IR/Entry.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

namespace IR {
	static char* names[] = {
		/* TypeNone		*/	"none	",
		/* TypeMove		*/	"move   ",
		/* TypeLoadImm	*/	"loadi  ",
		/* TypeAdd		*/	"add    ",
		/* TypeAddImm   */  "addi   ",
		/* TypeMult		*/	"mult   ",
		/* TypePrint	*/	"print  ",
		/* TypeEqual	*/	"equ    ",
		/* TypeNequal	*/	"neq    ",
		/* TypeLabel    */	"       ",
		/* TypeJump		*/	"jmp    ",
		/* TypeCJump	*/	"cjmp   ",
		/* TypeNCJump	*/	"ncjmp  ",
		/* TypePhi	    */	"phi    ",
		/* TypeCall     */  "call   ",
		/* TypeReturn   */  "return "
	};

	EntryThreeAddr::EntryThreeAddr(Type _type, Symbol *_lhs, Symbol *_rhs1, Symbol *_rhs2)
		: Entry(_type), lhs(_lhs), rhs1(_rhs1),	rhs2(_rhs2)
	{
	}

	EntryThreeAddr::~EntryThreeAddr()
	{
	}

	void EntryThreeAddr::print(const std::string &prefix)
	{
		bool needComma = false;
		printf("  %s%s", prefix.c_str(), names[type]);

		if(lhs) {
			printf("%s", lhs->name.c_str());
			needComma = true;
		}

		if(rhs1) {
			printf("%s%s", needComma ? ", " : "", rhs1->name.c_str());
			needComma = true;
		}

		if(rhs2)
			printf("%s%s", needComma ? ", " : "", rhs2->name.c_str());
	}

	void EntryThreeAddr::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		lhs = newSymbol;
	}

	Symbol *EntryThreeAddr::assign()
	{
		return lhs;
	}

	bool EntryThreeAddr::uses(Symbol *symbol)
	{
		return (rhs1 == symbol || rhs2 == symbol);
	}

	void EntryThreeAddr::replaceUse(Symbol *symbol, Symbol *newSymbol)
	{
		if(rhs1 == symbol) {
			rhs1 = newSymbol;
		}

		if(rhs2 == symbol) {
			rhs2 = newSymbol;
		}
	}

	EntryOneAddrImm::EntryOneAddrImm(Type _type, Symbol *_lhs, int _imm)
		: Entry(_type), lhs(_lhs), imm(_imm)
	{
	}

	EntryOneAddrImm::~EntryOneAddrImm()
	{
	}

	void EntryOneAddrImm::print(const std::string &prefix)
	{
		printf("  %s%s%s, %i", prefix.c_str(), names[type], lhs->name.c_str(), imm);
	}

	void EntryOneAddrImm::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		lhs = newSymbol;
	}

	Symbol *EntryOneAddrImm::assign()
	{
		return lhs;
	}

	EntryTwoAddrImm::EntryTwoAddrImm(Type _type, IR::Symbol *_lhs, IR::Symbol *_rhs, int _imm)
		: Entry(_type), lhs(_lhs), rhs(_rhs), imm(_imm)
	{
	}

	EntryTwoAddrImm::~EntryTwoAddrImm()
	{
	}

	void EntryTwoAddrImm::print(const std::string &prefix)
	{
		printf("  %s%s%s, %s, %i", prefix.c_str(), names[type], lhs->name.c_str(), rhs->name.c_str(), imm);
	}

	void EntryTwoAddrImm::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		lhs = newSymbol;
	}

	Symbol *EntryTwoAddrImm::assign()
	{
		return lhs;
	}

	bool EntryTwoAddrImm::uses(Symbol *symbol)
	{
		return (rhs == symbol);
	}

	void EntryTwoAddrImm::replaceUse(Symbol *symbol, Symbol *newSymbol)
	{
		if(rhs == symbol) {
			rhs = newSymbol;
		}
	}

	EntryLabel::EntryLabel(const std::string &_name)
		: Entry(TypeLabel), name(_name)
	{
	}

	void EntryLabel::print(const std::string &prefix)
	{
		printf("%s%s:", prefix.c_str(), name.c_str());
	}

	EntryJump::EntryJump(EntryLabel *_target)
		: Entry(TypeJump), target(_target)
	{
	}

	void EntryJump::print(const std::string &prefix)
	{
		printf("  %s%s%s", prefix.c_str(), names[type], target->name.c_str());
	}

	EntryCJump::EntryCJump(Symbol *_pred, EntryLabel *_trueTarget, EntryLabel *_falseTarget)
		: Entry(TypeCJump), pred(_pred), trueTarget(_trueTarget), falseTarget(_falseTarget)
	{
	}

	EntryCJump::~EntryCJump()
	{
	}

	void EntryCJump::print(const std::string &prefix)
	{
		printf("  %s%s%s, %s, %s", prefix.c_str(), names[type], pred->name.c_str(), trueTarget->name.c_str(), falseTarget->name.c_str());
	}

	bool EntryCJump::uses(Symbol *symbol)
	{
		return (pred == symbol);
	}

	void EntryCJump::replaceUse(Symbol *symbol, Symbol *newSymbol)
	{
		pred = newSymbol;
	}

	EntryPhi::EntryPhi(Symbol *_base, Symbol *_lhs, int _numArgs)
		: Entry(TypePhi), base(_base), lhs(_lhs), numArgs(_numArgs)
	{
		args = new Symbol*[numArgs];
		memset(args, 0, numArgs * sizeof(Symbol*));
	}

	EntryPhi::~EntryPhi()
	{
		delete[] args;
	}

	void EntryPhi::setArg(int num, Symbol *symbol)
	{
		args[num] = symbol;
	}

	void EntryPhi::removeArg(int num)
	{
		for(int i=num; i<numArgs - 1; i++) {
			args[i] = args[i+1];
		}

		numArgs--;
	}

	void EntryPhi::print(const std::string &prefix)
	{
		printf("  %s%s = PHI(", prefix.c_str(), lhs->name.c_str());
		for(int i=0; i<numArgs; i++) {
			printf("%s%s", args[i] ? args[i]->name.c_str() : "<none>", (i < numArgs - 1) ? ", " : "");
		}
		printf(")");
	}

	void EntryPhi::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		lhs = newSymbol;
	}

	bool EntryPhi::uses(Symbol *symbol)
	{
		for(int i=0; i<numArgs; i++) {
			if(args[i] == symbol)
				return true;
		}

		return false;
	}

	void EntryPhi::replaceUse(Symbol *symbol, Symbol *newSymbol)
	{
		for(int i=0; i<numArgs; i++) {
			if(args[i] == symbol)
			{
				args[i] = newSymbol;
				break;
			}
		}
	}

	Symbol *EntryPhi::assign()
	{
		return lhs;
	}

	EntryCall::EntryCall(Symbol *_lhs, Procedure *_target)
		: Entry(TypeCall), lhs(_lhs), target(_target)
	{
	}

	void EntryCall::print(const std::string &prefix)
	{
		if(lhs) {
			printf("  %s%s = %s%s()", prefix.c_str(), lhs->name.c_str(), names[type], target->name().c_str());
		} else {
			printf("  %s%s%s()", prefix.c_str(), names[type], target->name().c_str());
		}
	}

	Symbol *EntryCall::assign()
	{
		return lhs;
	}

}
