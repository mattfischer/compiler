#include "IR.h"

static char* names[] = {
	/* TypeLoad		*/	"load   ",
	/* TypeLoadImm	*/	"loadi  ",
	/* TypeAdd		*/	"add    ",
	/* TypeMult		*/	"mult   ",
	/* TypePrint	*/	"print  ",
	/* TypeEqual	*/	"equ    ",
	/* TypeNequal	*/	"neq    ",
	/* TypeJump		*/	"jmp    ",
	/* TypeCJump	*/	"cjmp   "
};

void IR::Entry::print(const std::string &prefix)
{
	if(type == TypePhi) {
		Phi *phi = (Phi*)this;
		printf("%s%s = PHI(", prefix.c_str(), phi->lhs->name.c_str());
		for(int i=0; i<phi->numArgs; i++) {
			Symbol *arg = phi->args[i];
			printf("%s%s", arg ? arg->name.c_str() : "<none>", (i < phi->numArgs - 1) ? ", " : "");
		}
		printf(")\n");
		return;
	}

	printf("%s%s ", prefix.c_str(), names[type]);

	switch(type) {
		case TypePrint:
		{
			ThreeAddr *threeAddr = (ThreeAddr*)this;
			printf("%s", threeAddr->lhs->name.c_str());
			break;
		}

		case TypeLoadImm:
		{
			Imm *imm = (Imm*)this;
			printf("%s, %i", imm->lhs->name.c_str(), imm->rhs);
			break;
		}

		case TypeLoad:
		{
			ThreeAddr *threeAddr = (ThreeAddr*)this;
			printf("%s, %s", threeAddr->lhs->name.c_str(),
							 threeAddr->rhs1->name.c_str());
			break;
		}

		case TypeAdd:
		case TypeMult:
		case TypeEqual:
		case TypeNequal:
		{
			ThreeAddr *threeAddr = (ThreeAddr*)this;
			printf("%s, %s, %s", threeAddr->lhs->name.c_str(),
								 threeAddr->rhs1->name.c_str(),
								 threeAddr->rhs2->name.c_str());
			break;
		}

		case TypeJump:
		{
			Jump *jump = (Jump*)this;
			printf("bb%i", jump->target->number);
			break;
		}

		case TypeCJump:
		{
			CJump *cJump = (CJump*)this;
			printf("%s, bb%i, bb%i", cJump->pred->name.c_str(),
								 cJump->trueTarget->number,
								 cJump->falseTarget->number);
			break;
		}
	}
	printf("\n");
}

IR::Entry::Class IR::Entry::getClass()
{
	switch(type) {
		case TypeAdd:
		case TypeMult:
		case TypeEqual:
		case TypeNequal:
		case TypeLoad:
			return ClassThreeAddr;

		case TypeLoadImm:
			return ClassImm;

		case TypeJump:
			return ClassJump;

		case TypeCJump:
			return ClassCJump;

		case TypePhi:
			return ClassPhi;
	}
}

IR::Entry *IR::Entry::newThreeAddr(Type type, Symbol *lhs, Symbol *rhs1, Symbol *rhs2)
{
	ThreeAddr *entry = new ThreeAddr;

	entry->type = type;
	entry->lhs = lhs;
	entry->rhs1 = rhs1;
	entry->rhs2 = rhs2;

	if(rhs1)
		rhs1->uses++;
	if(rhs2)
		rhs2->uses++;

	return (Entry*)entry;
}

IR::Entry *IR::Entry::newImm(Entry::Type type, Symbol *lhs, int rhs)
{
	Imm *entry = new Imm;

	entry->type = type;
	entry->lhs = lhs;
	entry->rhs = rhs;
	
	return (Entry*)entry;
}

IR::Entry *IR::Entry::newJump(Block *target)
{
	Jump *entry = new Jump;

	entry->type = TypeJump;
	entry->target = target;

	return (Entry*)entry;
}

IR::Entry *IR::Entry::newCJump(Symbol *pred, Block *trueTarget, Block *falseTarget)
{
	CJump *entry = new CJump;

	entry->type = TypeCJump;
	entry->pred = pred;
	entry->trueTarget = trueTarget;
	entry->falseTarget = falseTarget;
	
	pred->uses++;

	return (Entry*)entry;
}

IR::Entry *IR::Entry::newPhi(Symbol *base, Symbol *lhs, int numArgs)
{
	int size = sizeof(Phi) + (numArgs - 1) * sizeof(Symbol*);
	Phi *entry = (Phi*)new char[size];

	entry->type = TypePhi;
	entry->base = base;
	entry->lhs = lhs;
	entry->numArgs = numArgs;
	memset(entry->args, 0, numArgs * sizeof(Symbol*));

	return (Entry*)entry;
}

bool IR::Entry::assigns(Symbol *symbol)
{
	switch(getClass()) {
		case ClassThreeAddr:
			if(((ThreeAddr*)this)->lhs == symbol)
				return true;
			break;

		case ClassImm:
			if(((Imm*)this)->lhs == symbol)
				return true;
			break;

		case ClassPhi:
			if(((Phi*)this)->lhs == symbol)
				return true;
			break;

		default:
			break;
	}

	return false;
}

void IR::Entry::replaceAssign(Symbol *symbol, Symbol *newSymbol)
{
	switch(getClass()) {
		case ClassThreeAddr:
			((ThreeAddr*)this)->lhs = newSymbol;
			break;

		case ClassImm:
			((Imm*)this)->lhs = newSymbol;
			break;

		case ClassPhi:
			((Phi*)this)->lhs = newSymbol;

		default:
			break;
	}
}

bool IR::Entry::uses(Symbol *symbol)
{
	switch(getClass()) {
		case ClassThreeAddr:
			if(((ThreeAddr*)this)->rhs1 == symbol ||
			   ((ThreeAddr*)this)->rhs2 == symbol)
				return true;
			break;

		case ClassCJump:
			if(((CJump*)this)->pred == symbol)
				return true;
			break;

		case ClassPhi:
			{
				Phi *phi = (Phi*)this;
				for(int i=0; i<phi->numArgs; i++) {
					if(phi->args[i] == symbol)
						return true;
				}
				break;
			}

		default:
			break;
	}

	return false;
}

void IR::Entry::replaceUse(Symbol *symbol, Symbol *newSymbol)
{
	switch(getClass()) {
		case ClassThreeAddr:
			{
				ThreeAddr *threeAddr = (ThreeAddr*)this;

				if(threeAddr->rhs1 == symbol) {
					threeAddr->rhs1->uses--;
					threeAddr->rhs1 = newSymbol;
					newSymbol->uses++;
				}

				if(threeAddr->rhs2 == symbol) {
					threeAddr->rhs2->uses--;
					threeAddr->rhs2 = newSymbol;
					newSymbol->uses++;
				}
			}
			break;

		case ClassCJump:
			{
				CJump *cjump = (CJump*)this;

				cjump->pred->uses--;
				cjump->pred = newSymbol;
				newSymbol->uses++;
			}
			break;

		default:
			break;
	}
}

void IR::Entry::clear()
{
	switch(getClass()) {
		case ClassThreeAddr:
			{
				ThreeAddr *threeAddr = (ThreeAddr*)this;

				if(threeAddr->rhs1) {
					threeAddr->rhs1->uses--;
					threeAddr->rhs1 = 0;
				}

				if(threeAddr->rhs2) {
					threeAddr->rhs2->uses--;
					threeAddr->rhs2 = 0;
				}
			}
			break;

		case ClassCJump:
			{
				CJump *cjump = (CJump*)this;

				cjump->pred->uses--;
				cjump->pred = 0;
			}
			break;

		default:
			break;
	}
}
