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

		void print(const std::string &prefix = "");
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

		Block *idom;
		std::vector<Block*> domFrontiers;

		Block(int _number) : number(_number) {}

		void addPred(Block *block);
		void removePred(Block *block);

		void addSucc(Block *block);
		void removeSucc(Block *block);
	};

	class Procedure {
	public:
		Procedure(const std::string &name);

		void print(const std::string &prefix = "") const;
		
		Block *start() const { return mStart; }
		Block *end() const { return mEnd; }
		const std::string &name() const { return mName; }
		std::vector<Block*> &blocks() { return mBlocks; }

		void removeBlock(Block *block);

		Symbol *newTemp(Type *type);
		Symbol *addSymbol(const std::string &name, Type *type);
		Symbol *findSymbol(const std::string &name);
		Block *newBlock();

		void setCurrentBlock(Block *block);

		void computeDominance();

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

		Block *mCurrentBlock;

		void topologicalSort();
		void computeDominatorTree();
		void computeDominanceFrontiers();
	};

	IR();

	Procedure *main() { return mMain; }
	std::vector<Procedure*> procedures() { return mProcedures; }

	void print() const;

	void computeDominance();

private:
	std::vector<Procedure*> mProcedures;
	Procedure *mMain;
};
#endif