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
		printf("%sbb%i%s%s:\n", prefix.c_str(), block->number, (block == mStart)?" (start)" : "", (block == mEnd) ? " (end)" : "");
		for(Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next)
			entry->print(prefix + "  ");
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
		EntryJump *jump = (EntryJump*)entry;

		mCurrentBlock->succ.push_back(jump->target);
		jump->target->pred.push_back(mCurrentBlock);
	} else if(entry->type == Entry::TypeCJump) {
		EntryCJump *cjump = (EntryCJump*)entry;

		mCurrentBlock->succ.push_back(cjump->trueTarget);
		cjump->trueTarget->pred.push_back(mCurrentBlock);

		mCurrentBlock->succ.push_back(cjump->falseTarget);
		cjump->falseTarget->pred.push_back(mCurrentBlock);
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

void IR::Procedure::replaceEnd(Block *block)
{
	mEnd = block;
}