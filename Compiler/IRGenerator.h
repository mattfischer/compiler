#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "IR.h"
#include "SyntaxNode.h"

class IRGenerator {
public:
	IRGenerator(SyntaxNode *tree);

	struct List {
		std::vector<IRSymbol*> symbols;
		std::vector<IRLine*> lines;

		void print() const;
	};

	const List &generate();

private:
	SyntaxNode *mTree;
	int mNextTemp;
	List mList;

	void processNode(SyntaxNode *node);
	void emit(IRLine::Type type, IRSymbol *lhs) { emit(type, (long)lhs, 0, 0); }
	void emit(IRLine::Type type, IRSymbol *lhs, IRSymbol *rhs) { emit(type, (long)lhs, (long)rhs, 0); }
	void emit(IRLine::Type type, IRSymbol *lhs, IRSymbol *rhs1, IRSymbol *rhs2) { emit(type, (long)lhs, (long)rhs1, (long)rhs2); }
	void emit(IRLine::Type type, IRSymbol *lhs, int rhs) { emit(type, (long)lhs, (long)rhs, 0); }
	void emit(IRLine::Type type, long lhs, long rhs1, long rhs2);
	IRSymbol *processRValue(SyntaxNode *node);
	IRSymbol *newTemp(Type *type);
	IRSymbol *addSymbol(const std::string &name, Type *type);
	IRSymbol *findSymbol(const std::string &name);
};

#endif