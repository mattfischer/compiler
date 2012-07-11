#include "IR/Procedure.h"

#include "IR/Block.h"
#include "IR/Symbol.h"

#include "Front/Type.h"

#include <sstream>

namespace IR {
	Procedure::Procedure(const std::string &name)
	{
		mNextTemp = 0;

		mName = name;
		mStart = newBlock();
		mEnd = newBlock();
		mCurrentBlock = mStart;
	}

	void Procedure::print(const std::string &prefix) const
	{
		printf("%sSymbols:\n", prefix.c_str());
		for(SymbolList::const_iterator it = mSymbols.begin(); it != mSymbols.end(); it++) {
			Symbol *symbol = *it;
			printf("%s%s %s\n", (prefix + "  ").c_str(), symbol->type->name.c_str(), symbol->name.c_str());
		}
		printf("%s\n", prefix.c_str());
		printf("%sBody:\n", prefix.c_str());
		for(unsigned int i=0; i<mBlocks.size(); i++) {
			Block *block = mBlocks[i];
			printf("%sbb%i%s%s:\n", prefix.c_str(), block->number, (block == mStart)?" (start)" : "", (block == mEnd) ? " (end)" : "");
			for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				entry->print(prefix + "  ");
				printf("\n");
			}
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

	Symbol *Procedure::newTemp(Front::Type *type)
	{
		std::stringstream ss;
		ss << mNextTemp++;
		std::string name = "temp" + ss.str();

		return addSymbol(name, type);
	}

	Symbol *Procedure::addSymbol(const std::string &name, Front::Type *type)
	{
		Symbol *symbol = new Symbol(name, type);
		addSymbol(symbol);

		return symbol;
	}

	void Procedure::addSymbol(Symbol *symbol)
	{
		mSymbols.push_back(symbol);
	}

	Symbol *Procedure::findSymbol(const std::string &name)
	{
		for(SymbolList::iterator it = mSymbols.begin(); it != mSymbols.end(); it++) {
			Symbol *symbol = *it;
			if(symbol->name == name) {
				return symbol;
			}
		}

		return NULL;
	}

	Block *Procedure::newBlock()
	{
		Block *block = new Block((int)mBlocks.size());

		mBlocks.push_back(block);

		return block;
	}

	void Procedure::setCurrentBlock(Block *block)
	{
		mCurrentBlock = block;
	}

	void Procedure::emit(Entry *entry)
	{
		mCurrentBlock->entries.push_back(entry);
	}

	void Procedure::removeBlock(Block *block)
	{
		for(unsigned int i=0; i<mBlocks.size(); i++) {
			if(mBlocks[i] == block) {
				mBlocks.erase(mBlocks.begin() + i);
				break;
			}
		}

		delete block;
	}

	void Procedure::replaceEnd(Block *block)
	{
		mEnd = block;
	}
}