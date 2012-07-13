#include "IR/Entry.h"

#include "IR/Symbol.h"

namespace IR {
	static char* names[] = {
		/* TypeNone		*/	"none	",
		/* TypeLoad		*/	"load   ",
		/* TypeLoadImm	*/	"loadi  ",
		/* TypeAdd		*/	"add    ",
		/* TypeMult		*/	"mult   ",
		/* TypePrint	*/	"print  ",
		/* TypeEqual	*/	"equ    ",
		/* TypeNequal	*/	"neq    ",
		/* TypeLabel    */	"       ",
		/* TypeJump		*/	"jmp    ",
		/* TypeCJump	*/	"cjmp   "
	};

	EntryThreeAddr::EntryThreeAddr(Type _type, Symbol *_lhs, Symbol *_rhs1, Symbol *_rhs2)
		: Entry(_type), lhs(_lhs), rhs1(_rhs1),	rhs2(_rhs2)
	{
		if(lhs)
			lhs->addAssign(this);
		if(rhs1)
			rhs1->uses++;
		if(rhs2)
			rhs2->uses++;
	}

	EntryThreeAddr::~EntryThreeAddr()
	{
		if(lhs)
			lhs->removeAssign(this);

		if(rhs1)
			rhs1->uses--;

		if(rhs2)
			rhs2->uses--;
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

	bool EntryThreeAddr::assigns(Symbol *symbol)
	{
		return (lhs == symbol);
	}

	void EntryThreeAddr::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		if(lhs)
			lhs->removeAssign(this);

		lhs = newSymbol;
		lhs->addAssign(this);
	}

	bool EntryThreeAddr::uses(Symbol *symbol)
	{
		return (rhs1 == symbol || rhs2 == symbol);
	}

	void EntryThreeAddr::replaceUse(Symbol *symbol, Symbol *newSymbol)
	{
		if(rhs1 == symbol) {
			rhs1->uses--;
			rhs1 = newSymbol;
			newSymbol->uses++;
		}

		if(rhs2 == symbol) {
			rhs2->uses--;
			rhs2 = newSymbol;
			newSymbol->uses++;
		}
	}

	int EntryThreeAddr::value(bool &constant)
	{
		int result = 0;
		constant = false;

		switch(type) {
			case Entry::TypeAdd:
				if(rhs1->value && rhs2->value) {
					constant = true;
					result = *rhs1->value + *rhs2->value;
				}
				break;

			case Entry::TypeMult:
				if(rhs1->value && rhs2->value) {
					constant = true;
					result = *rhs1->value * *rhs2->value;
				}
				break;
				
			case Entry::TypeLoad:
				if(rhs1->value) {
					constant = true;
					result = *rhs1->value;
				}
				break;

			case Entry::TypeEqual:
				if(rhs1->value && rhs2->value) {
					constant = true;
					result = *rhs1->value == *rhs2->value;
				}
				break;

			case Entry::TypeNequal:
				if(rhs1->value && rhs2->value) {
					constant = true;
					result = *rhs1->value != *rhs2->value;
				}
				break;

			default:
				break;
		}

		return result;
	}

	Symbol *EntryThreeAddr::assignSymbol()
	{
		return lhs;
	}

	EntryImm::EntryImm(Symbol *_lhs, int _rhs)
		: Entry(TypeLoadImm), lhs(_lhs), rhs(_rhs)
	{
		if(lhs)
			lhs->addAssign(this);
	}

	EntryImm::~EntryImm()
	{
		if(lhs)
			lhs->removeAssign(this);
	}

	void EntryImm::print(const std::string &prefix)
	{
		printf("  %s%s%s, %i", prefix.c_str(), names[type], lhs->name.c_str(), rhs);
	}

	bool EntryImm::assigns(Symbol *symbol)
	{
		return (lhs == symbol);
	}

	void EntryImm::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		if(lhs)
			lhs->removeAssign(this);

		lhs = newSymbol;
		lhs->addAssign(this);
	}

	int EntryImm::value(bool &constant)
	{
		int result = 0;
		constant = false;

		switch(type) {
			case Entry::TypeLoadImm:
				constant = true;
				result = rhs;
				break;
		}

		return result;
	}

	Symbol *EntryImm::assignSymbol()
	{
		return lhs;
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
		pred->uses++;
	}

	EntryCJump::~EntryCJump()
	{
		pred->uses--;
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
		if(pred == symbol) {
			pred->uses--;
			pred = newSymbol;
			newSymbol->uses++;
		}
	}

	EntryPhi::EntryPhi(Symbol *_base, Symbol *_lhs, int _numArgs)
		: Entry(TypePhi), base(_base), lhs(_lhs), numArgs(_numArgs)
	{
		if(lhs)
			lhs->addAssign(this);
		args = new Symbol*[numArgs];
		memset(args, 0, numArgs * sizeof(Symbol*));
	}

	EntryPhi::~EntryPhi()
	{
		if(lhs)
			lhs->removeAssign(this);

		for(int i=0; i<numArgs; i++) {
			if(args[i])
				args[i]->uses--;
		}

		delete[] args;
	}

	void EntryPhi::setArg(int num, Symbol *symbol)
	{
		if(args[num])
			args[num]->uses--;

		args[num] = symbol;
		args[num]->uses++;
	}

	void EntryPhi::removeArg(int num)
	{
		args[num]->uses--;

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

	bool EntryPhi::assigns(Symbol *symbol)
	{
		return (lhs == symbol);
	}

	void EntryPhi::replaceAssign(Symbol *symbol, Symbol *newSymbol)
	{
		if(lhs)
			lhs->removeAssign(this);

		lhs = newSymbol;
		lhs->addAssign(this);
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
				args[i]->uses--;
				args[i] = newSymbol;
				newSymbol->uses++;
				break;
			}
		}
	}

	int EntryPhi::value(bool &constant)
	{
		int result = 0;
		constant = false;

		for(int i=0; i<numArgs; i++)
			if(!args[i]->value)
				return result;

		result = *args[0]->value;
		for(int i=1; i<numArgs; i++)
			if(*args[i]->value != result)
				return result;

		constant = true;
		return result;
	}

	Symbol *EntryPhi::assignSymbol()
	{
		return lhs;
	}
}