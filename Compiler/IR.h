#ifndef IR_H
#define IR_H

#include <vector>
#include <string>

#include "Type.h"

struct IR {
public:
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
		void *lhs;
		void *rhs1;
		void *rhs2;

		Entry(Type _type, void *_lhs, void *_rhs1, void *_rhs2) : type(_type), lhs(_lhs), rhs1(_rhs1), rhs2(_rhs2) {}

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

		void emit(Entry::Type type, void *lhs, void *rhs1 = 0, void *rhs2 = 0);

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