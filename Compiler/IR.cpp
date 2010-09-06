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

void IR::Entry::print(const std::string &prefix)
{
	if(type == TypePhi) {
		Phi *phi = (Phi*)this;
		printf("%s = PHI(", phi->lhs->name.c_str());
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

IR::Entry *IR::Entry::newThreeAddr(Type type, Symbol *lhs, Symbol *rhs1, Symbol *rhs2)
{
	ThreeAddr *entry = new ThreeAddr;

	entry->type = type;
	entry->lhs = lhs;
	entry->rhs1 = rhs1;
	entry->rhs2 = rhs2;

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

void IR::Block::addPred(Block *block)
{
	bool found = false;

	for(unsigned int i=0; i<pred.size(); i++) {
		if(pred[i] == block) {
			found = true;
			break;
		}
	}

	if(!found)
		pred.push_back(block);
}

void IR::Block::removePred(Block *block)
{
	for(unsigned int i=0; i<pred.size(); i++) {
		if(pred[i] == block) {
			pred.erase(pred.begin() + i);
			break;
		}
	}
}

void IR::Block::addSucc(Block *block)
{
	bool found = false;

	for(unsigned int i=0; i<succ.size(); i++) {
		if(succ[i] == block) {
			found = true;
			break;
		}
	}

	if(!found)
		succ.push_back(block);
}

void IR::Block::removeSucc(Block *block)
{
	for(unsigned int i=0; i<succ.size(); i++) {
		if(succ[i] == block) {
			succ.erase(succ.begin() + i);
			break;
		}
	}
}

IR::Procedure::Procedure(const std::string &name)
{
	mNextTemp = 0;

	mName = name;
	mStart = newBlock();
	mEnd = newBlock();
	mCurrentBlock = mStart;
}

void IR::Procedure::print(const std::string &prefix) const
{
	printf("%sSymbols:\n", prefix.c_str());
	for(unsigned int i=0; i<mSymbols.size(); i++) {
		printf("%s%s %s\n", (prefix + "  ").c_str(), mSymbols[i]->type->name.c_str(), mSymbols[i]->name.c_str());
	}
	printf("%s\n", prefix.c_str());
	printf("%sBody:\n", prefix.c_str());
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		Block *block = mBlocks[i];
		printf("%sbb%i%s:\n", prefix.c_str(), block->number, (block == mStart)?" (start)" : (block == mEnd) ? " (end)" : "");
		for(unsigned int j=0; j<block->entries.size(); j++)
			block->entries[j]->print(prefix + "  ");
	}
	printf("%s\n", prefix.c_str());

	/*	
	printf("Graph:\n");
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		Block *block = mBlocks[i];
		printf("%i -> ", block->number);
		for(unsigned int j=0; j<block->succ.size(); j++)
			printf("%i ", block->succ[j]->number);
		printf("\n");
	}
	printf("\n");
	*/

	/*	
	printf("Dominator tree:\n");
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		Block *block = mBlocks[i];

		printf("%i -> %i\n", block->number, block->idom->number);
	}
	printf("\n");
	*/

	/*
	printf("Dominance frontiers:\n");
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		Block *block = mBlocks[i];

		printf("%i -> ", block->number);
		for(unsigned int j=0; j<block->domFrontiers.size(); j++)
			printf("%i ", block->domFrontiers[j]->number);
		printf("\n");
	}
	printf("\n");
	*/
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
	Block *block = new Block((int)mBlocks.size());
	mBlocks.push_back(block);

	return block;
}

void IR::Procedure::setCurrentBlock(Block *block)
{
	mCurrentBlock = block;
}

void IR::Procedure::emit(Entry *entry)
{
	mCurrentBlock->appendEntry(entry);

	if(entry->type == Entry::TypeJump) {
		Entry::Jump *jump = (Entry::Jump*)entry;

		mCurrentBlock->succ.push_back(jump->target);
		jump->target->pred.push_back(mCurrentBlock);
	} else if(entry->type == Entry::TypeCJump) {
		Entry::CJump *cjump = (Entry::CJump*)entry;

		mCurrentBlock->succ.push_back(cjump->trueTarget);
		cjump->trueTarget->pred.push_back(mCurrentBlock);

		mCurrentBlock->succ.push_back(cjump->falseTarget);
		cjump->falseTarget->pred.push_back(mCurrentBlock);
	}
}

void IR::Procedure::computeDominance()
{
	topologicalSort();
	computeDominatorTree();
	computeDominanceFrontiers();
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

void IR::Procedure::topologicalSort()
{
	std::vector<bool> seen(mBlocks.size());
	std::vector<int> output;
	std::vector<IR::Block*> newBlocks = mBlocks;

	for(unsigned int i=0; i<mBlocks.size(); i++)
		mBlocks[i]->number = i;

	topoSortRecurse(start(), seen, output);

	mBlocks.clear();
	for(unsigned int i=0; i<newBlocks.size(); i++) {
		Block *block = newBlocks[output[newBlocks.size() - i - 1]];
		mBlocks.push_back(block);
		block->number = i;
	}
}

static IR::Block *domIntersect(IR::Block *a, IR::Block *b)
{
	while(a != b) {
		while(a->number > b->number)
			a = a->idom;
		while(b->number > a->number)
			b = b->idom;
	}

	return a;
}

void IR::Procedure::computeDominatorTree()
{
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		mBlocks[i]->idom = 0;
	}
	mBlocks[0]->idom = mBlocks[0];

	bool changed;
	do {
		changed = false;
		for(unsigned int i=1; i<mBlocks.size(); i++) {
			Block *block = mBlocks[i];
			Block *newIdom = 0;
			for(unsigned int j=0; j<block->pred.size(); j++) {
				Block *pred = block->pred[j];

				if(pred->idom != 0) {
					if(newIdom)
						newIdom = domIntersect(pred, newIdom);
					else
						newIdom = pred;
				}
			}

			if(newIdom != block->idom) {
				block->idom = newIdom;
				changed = true;
			}
		}
	} while(changed);
}

void IR::Procedure::computeDominanceFrontiers()
{
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		Block *block = mBlocks[i];

		if(block->pred.size() < 2)
			continue;

		for(unsigned int j=0; j<block->pred.size(); j++) {
			Block *runner = block->pred[j];
			while(runner != block->idom) {
				bool found = false;
				for(unsigned int k=0; k<runner->domFrontiers.size(); k++) {
					if(runner->domFrontiers[k] == block) {
						found = true;
						break;
					}
				}

				if(!found) {
					runner->domFrontiers.push_back(block);
				}

				runner = runner->idom;
			}
		}
	}
}

void IR::Procedure::removeBlock(Block *block)
{
	for(unsigned int i=0; i<mBlocks.size(); i++) {
		if(mBlocks[i] == block) {
			mBlocks.erase(mBlocks.begin() + i);
			break;
		}
	}

	delete block;
}

IR::IR()
{
	mMain = new Procedure("main");
	mProcedures.push_back(mMain);
}

void IR::print() const
{
	for(unsigned int i=0; i<mProcedures.size(); i++) {
		printf("<%s>\n", mProcedures[i]->name().c_str());
		mProcedures[i]->print("  ");
	}
}

void IR::computeDominance()
{
	for(unsigned int i=0; i<mProcedures.size(); i++) {
		mProcedures[i]->computeDominance();
	}
}