#include "IR/Entry.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

namespace IR {
	static const char* names[] = {
		/* TypeNone		  */ "none	   ",
		/* TypeMove		  */ "move     ",
		/* TypeAdd		  */ "add      ",
		/* TypeSubtract	  */ "sub      ",
		/* TypeMult		  */ "mult     ",
		/* TypeDivide	  */ "div      ",
		/* TypeModulo	  */ "mod      ",
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
		/* TypeCallIndirect*/ "calli    ",
		/* TypeLoadRet    */ "ldret    ",
		/* TypeStoreRet   */ "stret    ",
		/* TypeLoadArg    */ "ldarg    ",
		/* TypeStoreArg   */ "starg    ",
		/* TypeLoadStack  */ "ldstk    ",
		/* TypeStoreStack */ "ststk    ",
		/* TypeLoadAddress*/ "ldaddr   ",
		/* TypePrologue   */ "prologue ",
		/* TypeEpilogue   */ "epilogue ",
		/* TypeNew        */ "new      ",
		/* TypeStoreMem   */ "stmem    ",
		/* TypeLoadMem    */ "ldmem    ",
		/* TypeLoadString */ "ldstr    ",
		/* TypeFunctionAddr */ "addr     "
	};

	static bool lhsAssign(Entry::Type type)
	{
		switch(type) {
			case Entry::Type::StoreMem:
				return false;

			default:
				return true;
		}
	}

	EntryThreeAddr::EntryThreeAddr(Type _type, const Symbol *_lhs, const Symbol *_rhs1, const Symbol *_rhs2, int _imm)
		: Entry(_type), lhs(_lhs), rhs1(_rhs1),	rhs2(_rhs2), imm(_imm)
	{
	}

	EntryThreeAddr::~EntryThreeAddr()
	{
	}

	void EntryThreeAddr::print(std::ostream &o) const
	{
		bool needComma = false;
		o << "  " << names[(int)type];

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

	void EntryThreeAddr::replaceAssign(const Symbol *symbol, const Symbol *newSymbol)
	{
		if(lhsAssign(type)) {
			lhs = newSymbol;
		}
	}

	const Symbol *EntryThreeAddr::assign() const
	{
		return lhsAssign(type) ? lhs : 0;
	}

	bool EntryThreeAddr::uses(const Symbol *symbol) const
	{
		return (rhs1 == symbol || rhs2 == symbol || (!lhsAssign(type) && lhs == symbol));
	}

	void EntryThreeAddr::replaceUse(const Symbol *symbol, const Symbol *newSymbol)
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
		: Entry(Type::Label), name(_name)
	{
	}

	void EntryLabel::print(std::ostream &o) const
	{
		o << name << ":";
	}

	EntryJump::EntryJump(EntryLabel *_target)
		: Entry(Type::Jump), target(_target)
	{
	}

	void EntryJump::print(std::ostream &o) const
	{
		o << "  " << names[(int)type] << target->name;
	}

	EntryCJump::EntryCJump(const Symbol *_pred, EntryLabel *_trueTarget, EntryLabel *_falseTarget)
		: Entry(Type::CJump), pred(_pred), trueTarget(_trueTarget), falseTarget(_falseTarget)
	{
	}

	EntryCJump::~EntryCJump()
	{
	}

	void EntryCJump::print(std::ostream &o) const
	{
		o << "  " << names[(int)type] << pred->name << ", " << trueTarget->name << ", " << falseTarget->name;
	}

	bool EntryCJump::uses(const Symbol *symbol) const
	{
		return (pred == symbol);
	}

	void EntryCJump::replaceUse(const Symbol *symbol, const Symbol *newSymbol)
	{
		pred = newSymbol;
	}

	EntryPhi::EntryPhi(const Symbol *_base, const Symbol *_lhs, int _numArgs)
		: Entry(Type::Phi), base(_base), lhs(_lhs), numArgs(_numArgs)
	{
		args = new const Symbol*[numArgs];
		memset(args, 0, numArgs * sizeof(const Symbol*));
	}

	EntryPhi::~EntryPhi()
	{
		delete[] args;
	}

	void EntryPhi::setArg(int num, const Symbol *symbol)
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

	void EntryPhi::replaceAssign(const Symbol *symbol, const Symbol *newSymbol)
	{
		lhs = newSymbol;
	}

	bool EntryPhi::uses(const Symbol *symbol) const
	{
		for(int i=0; i<numArgs; i++) {
			if(args[i] == symbol)
				return true;
		}

		return false;
	}

	void EntryPhi::replaceUse(const Symbol *symbol, const Symbol *newSymbol)
	{
		for(int i=0; i<numArgs; i++) {
			if(args[i] == symbol)
			{
				args[i] = newSymbol;
				break;
			}
		}
	}

	const Symbol *EntryPhi::assign() const
	{
		return lhs;
	}

	EntryCall::EntryCall(Type _type, const std::string &_target)
		: Entry(_type), target(_target)
	{
	}

	void EntryCall::print(std::ostream &o) const
	{
		o << "  " << names[(int)type] << target;
	}

	EntryString::EntryString(Type _type, const Symbol *_lhs, const std::string &_string)
		: Entry(_type), lhs(_lhs), string(_string)
	{
	}

	void EntryString::print(std::ostream &o) const
	{
		o << "  " << names[(int)type] << lhs->name << ", \"" << string << "\"";
	}

	const Symbol *EntryString::assign() const
	{
		return lhs;
	}

	void EntryString::replaceAssign(const Symbol *symbol, const Symbol *newSymbol)
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
