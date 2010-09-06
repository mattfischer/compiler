#include "IR.h"

#include <sstream>

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
	addSymbol(symbol);

	return symbol;
}

void IR::Procedure::addSymbol(Symbol *symbol)
{
	mSymbols.push_back(symbol);
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
