#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "IR.h"
#include "SyntaxNode.h"
#include "Type.h"

#include <string>

class IRGenerator {
public:
	IRGenerator(SyntaxNode *tree);

	struct List {
		struct Symbol {
			std::string name;
			Type *type;

			Symbol(const std::string &_name, Type *_type) : name(_name), type(_type) {}
		};

		std::vector<Symbol*> symbols;

		struct Block {
			int number;
			std::vector<IRLine*> lines;
			std::vector<Block*> pred;
			std::vector<Block*> succ;

			Block(int _number) : number(_number) {}
		};

		std::vector<Block*> blocks;
		Block *start;
		Block *end;

		void print() const;
		void printGraph() const;
	};

	const List &generate();

private:
	SyntaxNode *mTree;
	int mNextTemp;
	List::Block *mCurrentBlock;
	int mNextBlock;
	List mList;

	void processNode(SyntaxNode *node);
	void emit(IRLine::Type type, void *lhs, void *rhs1 = 0, void *rhs2 = 0);
	List::Symbol *processRValue(SyntaxNode *node);
	List::Symbol *newTemp(Type *type);
	List::Symbol *addSymbol(const std::string &name, Type *type);
	List::Symbol *findSymbol(const std::string &name);
	List::Block *newBlock();
	void setCurrentBlock(List::Block *block);
	void topoSortRecurse(List::Block *block, std::vector<bool> &seen, std::vector<int> &output);
	void topoSort();

};

#endif