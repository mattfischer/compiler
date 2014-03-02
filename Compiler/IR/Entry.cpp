#include "IR/Entry.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

namespace IR {
	static char* names[] = {
		/* TypeNone		  */ "none	   ",
		/* TypeMove		  */ "move     ",
		/* TypeAdd		  */ "add      ",
		/* TypeSubtract	  */ "sub      ",
		/* TypeMult		  */ "mult     ",
		/* TypePrint      */ "print    ",
		/* TypeEqual	  */ "equ      ",
		/* TypeNequal	  */ "neq      ",
		/* TypeLessThan	  */ "lt       ",
		/* TypeLessThanE  */ "lte      ",
		/* TypeGreaterThan*/ "gt       ",
		/* TypeGreaterThanE*/ "gte     ",
		/* TypeOr         */ "or       ",
		/* TypeAnd        */ "and      ",
		/* TypeLabel      */ "         ",
		/* TypeJump		  */ "jmp      ",
		/* TypeCJump	  */ "cjmp     ",
		/* TypePhi	      */ "phi      ",
		/* TypeCall       */ "call     ",
		/* TypeLoadRet    */ "ldret    ",
		/* TypeStoreRet   */ "stret    ",
		/* TypeLoadArg    */ "ldarg    ",
		/* TypeStoreArg   */ "starg    ",
		/* TypeLoadStack  */ "ldstk    ",
		/* TypeStoreStack */ "ststk    ",
		/* TypePrologue   */ "prologue ",
		/* TypeEpilogue   */ "epilogue ",
		/* TypeNew        */ "new      ",
		/* TypeStoreMem   */ "stmem    ",
		/* TypeLoadMem    */ "ldmem    ",
		/* TypeStoreMemByte*/ "stmemb   ",
		/* TypeLoadMemByte*/ "ldmemb   ",
		/* TypeLoadString */ "ldstr    ",
		/* TypeConcat     */ "concat   ",
		/* TypeStringBool */ "strbool  ",
		/* TypeStringInt  */ "strint   ",
	};

	static bool lhsAssign(Entry::Type type)
	{
		switch(type) {
			case Entry::TypeStoreMem:
			case Entry::TypeStoreMemByte:
				return false;

			default:
				return true;
		}
	}

	EntryThreeAddr::EntryThreeAddr(Type _type, Symbol *_lhs, Symbol *_rhs1, Symbol *_rhs2, int _imm)
		: Entry(_type), lhs(_lhs), rhs1(_rhs1),	rhs2(_rhs2), imm(_imm)
	{
	}

	EntryThreeAddr::~EntryThreeAddr()
	{
	}

	void EntryThreeAddr::print(std::ostream &o) const
	{
		bool needComma = false;
		o << "  " << names[type];

		if(lhs) {
			o << lhs->name;
			needComma = true;
		}

		if(rhs1) {
			o << (needComma ? ", " : "") << rhs1->name;
			needComma = true;
		}

		if(rhs2) {
			o << (needComma ? ", " : "") << rhs2->name;
			needComma = true;
		}

		if(imm || !rhs1) {
			o << (needComma ? ", " : "") << imm;
		}
	}

	void EntryThreeAddr::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		if(lhsAssign(type)) {
			lhs = newSymbol;
		}
	}

	Symbol *EntryThreeAddr::assign()
	{
		return lhsAssign(type) ? lhs : 0;
	}

	bool EntryThreeAddr::uses(Symbol *symbol)
	{
		return (rhs1 == symbol || rhs2 == symbol || (!lhsAssign(type) && lhs == symbol));
	}

	void EntryThreeAddr::replaceUse(Symbol *symbol, Symbol *newSymbol)
	{
		if(rhs1 == symbol) {
			rhs1 = newSymbol;
		}

		if(rhs2 == symbol) {
			rhs2 = newSymbol;
		}

		if(!lhsAssign(type) && lhs == symbol) {
			lhs = newSymbol;
		}
	}

	EntryLabel::EntryLabel(const std::string &_name)
		: Entry(TypeLabel), name(_name)
	{
	}

	void EntryLabel::print(std::ostream &o) const
	{
		o << name << ":";
	}

	EntryJump::EntryJump(EntryLabel *_target)
		: Entry(TypeJump), target(_target)
	{
	}

	void EntryJump::print(std::ostream &o) const
	{
		o << "  " << names[type] << target->name;
	}

	EntryCJump::EntryCJump(Symbol *_pred, EntryLabel *_trueTarget, EntryLabel *_falseTarget)
		: Entry(TypeCJump), pred(_pred), trueTarget(_trueTarget), falseTarget(_falseTarget)
	{
	}

	EntryCJump::~EntryCJump()
	{
	}

	void EntryCJump::print(std::ostream &o) const
	{
		o << "  " << names[type] << pred->name << ", " << trueTarget->name << ", " << falseTarget->name;
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

	void EntryPhi::print(std::ostream &o) const
	{
		o << "  " << lhs->name << " = PHI(";
		for(int i=0; i<numArgs; i++) {
			o << (args[i] ? args[i]->name.c_str() : "<none>") << ((i < numArgs - 1) ? ", " : "");
		}
		o << ")";
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

	EntryCall::EntryCall(Procedure *_target)
		: Entry(TypeCall), target(_target)
	{
	}

	void EntryCall::print(std::ostream &o) const
	{
		o << "  " << names[type] << target->name();
	}

	EntryString::EntryString(Type _type, Symbol *_lhs, const std::string &_string)
		: Entry(_type), lhs(_lhs), string(_string)
	{
	}

	void EntryString::print(std::ostream &o) const
	{
		o << "  " << names[type] << lhs->name << ", \"" << string << "\"";
	}

	Symbol *EntryString::assign()
	{
		return lhs;
	}

	void EntryString::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		if(lhs == symbol) {
			lhs = newSymbol;
		}
	}

	std::ostream &operator<<(std::ostream &o, const Entry &entry)
	{
		entry.print(o);

		return o;
	}
}
