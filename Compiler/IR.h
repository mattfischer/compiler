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
			TypeNCJump,
			TypePhi
		};

		Type type;

		Entry(Type _type) : type(_type) {}
		virtual ~Entry() {}

		virtual void print(const std::string &prefix = "") {}

		virtual bool assigns(Symbol *symbol) { return false; }
		virtual bool uses(Symbol *symbol) { return false; }
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol) {}
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol) {}
	};

	struct EntryThreeAddr : public Entry {
		Symbol *lhs;
		Symbol *rhs1;
		Symbol *rhs2;

		EntryThreeAddr(Type _type, Symbol *_lhs, Symbol *_rhs1 = 0, Symbol *_rhs2 = 0);
		virtual ~EntryThreeAddr();

		virtual void print(const std::string &prefix);

		virtual bool assigns(Symbol *symbol);
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	struct EntryImm : public Entry {
		Symbol *lhs;
		int rhs;

		EntryImm(Type _type, Symbol *_lhs, int _rhs);

		virtual void print(const std::string &prefix);

		virtual bool assigns(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
	};

	struct EntryJump : public Entry {
		Block *target;

		EntryJump(Block *_target);

		virtual void print(const std::string &prefix);
	};

	struct EntryCJump : public Entry {
		Symbol *pred;
		Block *trueTarget;
		Block *falseTarget;

		EntryCJump(Symbol *_pred, Block *_trueTarget, Block *_falseTarget);
		virtual ~EntryCJump();

		virtual void print(const std::string &prefix);

		virtual bool uses(Symbol *symbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	struct EntryPhi : public Entry {
		Symbol *base;
		Symbol *lhs;
		int numArgs;
		Symbol **args;

		EntryPhi(Symbol *_base, Symbol *_lhs, int _numArgs);
		virtual ~EntryPhi();

		virtual void print(const std::string &prefix);

		virtual bool assigns(Symbol *symbol);
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	struct Symbol {
		std::string name;
		Type *type;
		int uses;

		Symbol(const std::string &_name, Type *_type) : name(_name), type(_type), uses(0) {}
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

		void appendEntry(Entry *entry) { entries.push_back(entry); }
		void prependEntry(Entry *entry) { entries.insert(entries.begin(), entry); }

		Entry *head() const { return entries.size() > 0 ? entries[0] : 0; }
		Entry *tail() const { return entries.size() > 0 ? entries[entries.size() - 1] : 0; }
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

		std::vector<Symbol*> &symbols() { return mSymbols; }
		Symbol *newTemp(Type *type);
		Symbol *addSymbol(const std::string &name, Type *type);
		void addSymbol(Symbol *symbol);
		Symbol *findSymbol(const std::string &name);
		Block *newBlock();

		void setCurrentBlock(Block *block);

		void computeDominance();

		void emit(Entry *entry);

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