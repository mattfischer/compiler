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
	printf("%s ", names[type]);

	switch(type) {
		case TypePrint:
			printf("%s", ((Symbol*)lhs)->name.c_str());
			break;

		case TypeLoadImm:
			printf("%s, %i", ((Symbol*)lhs)->name.c_str(), (int)rhs1);
			break;

		case TypeLoad:
			printf("%s, %s", ((Symbol*)lhs)->name.c_str(),
							 ((Symbol*)rhs1)->name.c_str());
			break;

		case TypeAdd:
		case TypeMult:
		case TypeEqual:
		case TypeNequal:
			printf("%s, %s, %s", ((Symbol*)lhs)->name.c_str(),
								 ((Symbol*)rhs1)->name.c_str(),
								 ((Symbol*)rhs2)->name.c_str());
			break;

		case TypeJump:
			printf("bb%i", ((Block*)lhs)->number);
			break;

		case TypeCJump:
			printf("%s, bb%i, bb%i", ((Symbol*)lhs)->name.c_str(),
								 ((Block*)rhs1)->number,
								 ((Block*)rhs2)->number);
			break;
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

void IR::Procedure::emit(Entry::Type type, void *lhs, void *rhs1, void *rhs2)
{
	Entry *entry = new Entry(type, lhs, rhs1, rhs2);
	mCurrentBlock->entries.push_back(entry);

	if(type == IR::Entry::TypeJump) {
		IR::Block *block = (IR::Block*)lhs;
		mCurrentBlock->succ.push_back(block);
		block->pred.push_back(mCurrentBlock);
	} else if(type == IR::Entry::TypeCJump) {
		IR::Block *block = (IR::Block*)rhs1;
		mCurrentBlock->succ.push_back(block);
		block->pred.push_back(mCurrentBlock);

		block = (IR::Block*)rhs2;
		mCurrentBlock->succ.push_back(block);
		block->pred.push_back(mCurrentBlock);
	}
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
		printf("%s:\n", mProcedures[i]->name());
		mProcedures[i]->print();
		printf("\n");
		mProcedures[i]->printGraph();
		printf("\n");
	}
}