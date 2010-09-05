#include "IR.h"

#include <sstream>

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

void IR::Entry::print()
{
	Imm *imm = (Imm*)this;
	printf("%s ", names[type]);

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

IR::Procedure::Procedure(const std::string &name)
{
	mNextTemp = 0;
	mNextBlock = 0;

	mName = name;
	mStart = newBlock();
	mEnd = newBlock();
	mCurrentBlock = mStart;
}

void IR::Procedure::print() const
{
	printf("Symbols:\n");
	for(unsigned int i=0; i<mSymbols.size(); i++) {
		printf("%s %s\n", mSymbols[i]->type->name.c_str(), mSymbols[i]->name.c_str());
	}
	printf("\n");
	printf("Body:\n");
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		Block *block = mBlocks[i];
		printf("bb%i%s:\n", block->number, (block == mStart)?" (start)" : (block == mEnd) ? " (end)" : "");
		for(unsigned int j=0; j<block->entries.size(); j++) {
			printf("  ");
			block->entries[j]->print();
		}
	}
}

void IR::Procedure::printGraph() const
{
	printf("Graph:\n");
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		Block *block = mBlocks[i];
		printf("%i -> ", block->number);
		for(unsigned int j=0; j<block->succ.size(); j++)
			printf("%i ", block->succ[j]->number);
		printf("\n");
	}
}


IR::Symbol *IR::Procedure::newTemp(Type *type)
{
	std::stringstream ss;
	ss << mNextTemp++;
	std::string name = "temp" + ss.str();

	return addSymbol(name, type);
}

IR::Symbol *IR::Procedure::addSymbol(const std::string &name, Type *type)
{
	Symbol *symbol = new Symbol(name, type);
	mSymbols.push_back(symbol);

	return symbol;
}

IR::Symbol *IR::Procedure::findSymbol(const std::string &name)
{
	for(unsigned int i=0; i<mSymbols.size(); i++) {
		if(mSymbols[i]->name == name) {
			return mSymbols[i];
		}
	}

	return NULL;
}

IR::Block *IR::Procedure::newBlock()
{
	Block *block = new Block(mNextBlock++);
	mBlocks.push_back(block);

	return block;
}

void IR::Procedure::setCurrentBlock(Block *block)
{
	mCurrentBlock = block;
}

void IR::Procedure::emitThreeAddr(Entry::Type type, Symbol *lhs, Symbol *rhs1, Symbol *rhs2)
{
	IR::Entry::ThreeAddr *entry = new IR::Entry::ThreeAddr;

	entry->type = type;
	entry->lhs = lhs;
	entry->rhs1 = rhs1;
	entry->rhs2 = rhs2;

	mCurrentBlock->entries.push_back((Entry*)entry);
}

void IR::Procedure::emitImm(Entry::Type type, Symbol *lhs, int rhs)
{
	IR::Entry::Imm *entry = new IR::Entry::Imm;

	entry->type = type;
	entry->lhs = lhs;
	entry->rhs = rhs;
	
	mCurrentBlock->entries.push_back((Entry*)entry);
}

void IR::Procedure::emitJump(Block *target)
{
	IR::Entry::Jump *entry = new IR::Entry::Jump;

	entry->type = Entry::TypeJump;
	entry->target = target;

	mCurrentBlock->entries.push_back((Entry*)entry);
	mCurrentBlock->succ.push_back(target);
	target->pred.push_back(mCurrentBlock);
}

void IR::Procedure::emitCJump(Symbol *pred, Block *trueTarget, Block *falseTarget)
{
	IR::Entry::CJump *entry = new IR::Entry::CJump;

	entry->type = Entry::TypeCJump;
	entry->pred = pred;
	entry->trueTarget = trueTarget;
	entry->falseTarget = falseTarget;
	
	mCurrentBlock->entries.push_back((Entry*)entry);
	mCurrentBlock->succ.push_back(trueTarget);
	trueTarget->pred.push_back(mCurrentBlock);

	mCurrentBlock->succ.push_back(falseTarget);
	falseTarget->pred.push_back(mCurrentBlock);
}

static void topoSortRecurse(IR::Block *block, std::vector<bool> &seen, std::vector<int> &output)
{
	if(seen[block->number]) 
		return;

	seen[block->number] = true;

	for(unsigned int i=0; i<block->succ.size(); i++)
		topoSortRecurse(block->succ[i], seen, output);

	output.push_back(block->number);
}

void IR::Procedure::topoSort()
{
	std::vector<bool> seen(mBlocks.size());
	std::vector<int> output;
	std::vector<IR::Block*> newBlocks = mBlocks;

	topoSortRecurse(start(), seen, output);

	mBlocks.clear();
	for(unsigned int i=0; i<newBlocks.size(); i++) {
		Block *block = newBlocks[output[newBlocks.size() - i - 1]];
		mBlocks.push_back(block);
		block->number = i;
	}
}

IR::IR()
{
	mMain = new Procedure("main");
	mProcedures.push_back(mMain);
}

void IR::print() const
{
	for(unsigned int i=0; i<mProcedures.size(); i++) {
		printf("%s:\n", mProcedures[i]->name().c_str());
		mProcedures[i]->print();
		printf("\n");
		mProcedures[i]->printGraph();
		printf("\n");
	}
}