#ifndef IR_H
#define IR_H

#include <vector>
#include <string>

#include "Type.h"

struct IR {
public:
	struct Symbol;
	struct Block;

	struct Entry {
		enum Type {
			TypeLoad,
			TypeLoadImm,
			TypeAdd,
			TypeMult,
			TypePrint,
			TypeEqual,
			TypeNequal,
			TypeJump,
			TypeCJump,
			TypeNCJump
		};

		Type type;

		struct ThreeAddr {
			Type type;
			Symbol *lhs;
			Symbol *rhs1;
			Symbol *rhs2;
		};

		struct Imm {
			Type type;
			Symbol *lhs;
			int rhs;
		};

		struct Jump {
			Type type;
			Block *target;
		};

		struct CJump {
			Type type;
			Symbol *pred;
			Block *trueTarget;
			Block *falseTarget;
		};

		void print();
	};

	struct Symbol {
		std::string name;
		Type *type;

		Symbol(const std::string &_name, Type *_type) : name(_name), type(_type) {}
	};

	struct Block {
		int number;
		std::vector<Entry*> entries;
		std::vector<Block*> pred;
		std::vector<Block*> succ;

		Block(int _number) : number(_number) {}
	};

	class Procedure {
	public:
		Procedure(const std::string &name);

		void print() const;
		void printGraph() const;
		
		Block *start() const { return mStart; }
		Block *end() const { return mEnd; }
		const std::string &name() const { return mName; }

		Symbol *newTemp(Type *type);
		Symbol *addSymbol(const std::string &name, Type *type);
		Symbol *findSymbol(const std::string &name);
		Block *newBlock();

		void setCurrentBlock(Block *block);

		void topoSort();

		void emitThreeAddr(Entry::Type type, Symbol *lhs, Symbol *rhs1 = 0, Symbol *rhs2 = 0);
		void emitImm(Entry::Type type, Symbol *lhs, int rhs);
		void emitJump(Block *target);
		void emitCJump(Symbol *pred, Block *trueTarget, Block *falseTarget);

	private:
		std::string mName;
		std::vector<Symbol*> mSymbols;
		std::vector<Block*> mBlocks;
		Block *mStart;
		Block *mEnd;

		int mNextTemp;
		int mNextBlock;

		Block *mCurrentBlock;
	};

	IR();

	Procedure *main() { return mMain; }

	void print() const;

private:
	std::vector<Procedure*> mProcedures;
	Procedure *mMain;
};
#endif