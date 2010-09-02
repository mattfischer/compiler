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
			std::string name;
			std::vector<IRLine*> lines;

			Block(const std::string &_name) : name(_name) {}
		};

		std::vector<Block*> blocks;

		void print() const;
	};

	const List &generate();

private:
	SyntaxNode *mTree;
	int mNextTemp;
	List::Block *mCurrentBlock;
	int mNextBlock;
	List mList;

	void processNode(SyntaxNode *node);
	void emit(IRLine::Type type, List::Block *lhs) { emit(type, (long)lhs, 0, 0); }
	void emit(IRLine::Type type, List::Symbol *lhs, List::Block *rhs) { emit(type, (long)lhs, (long)rhs, 0); }
	void emit(IRLine::Type type, List::Symbol *lhs) { emit(type, (long)lhs, 0, 0); }
	void emit(IRLine::Type type, List::Symbol *lhs, List::Symbol *rhs) { emit(type, (long)lhs, (long)rhs, 0); }
	void emit(IRLine::Type type, List::Symbol *lhs, List::Symbol *rhs1, List::Symbol *rhs2) { emit(type, (long)lhs, (long)rhs1, (long)rhs2); }
	void emit(IRLine::Type type, List::Symbol *lhs, int rhs) { emit(type, (long)lhs, (long)rhs, 0); }
	void emit(IRLine::Type type, long lhs, long rhs1, long rhs2);
	List::Symbol *processRValue(SyntaxNode *node);
	List::Symbol *newTemp(Type *type);
	List::Symbol *addSymbol(const std::string &name, Type *type);
	List::Symbol *findSymbol(const std::string &name);
	List::Block *newBlock();
	void setCurrentBlock(List::Block *block);
};

#endif