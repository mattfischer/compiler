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
			TypeNone,
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

		Entry *prev;
		Entry *next;

		void insertAfter(Entry *entry);
		void insertBefore(Entry *entry);
		void remove();
		void replace(Entry *entry);

		Entry(Type _type) : type(_type), prev(0), next(0) {}
		virtual ~Entry() {}

		virtual void print(const std::string &prefix = "") {}

		virtual bool assigns(Symbol *symbol) { return false; }
		virtual bool uses(Symbol *symbol) { return false; }
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol) {}
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol) {}
		virtual int value(bool &constant) { constant = false; return 0; }
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
		virtual int value(bool &constant);
	};

	struct EntryImm : public Entry {
		Symbol *lhs;
		int rhs;

		EntryImm(Type _type, Symbol *_lhs, int _rhs);
		virtual ~EntryImm();

		virtual void print(const std::string &prefix);

		virtual bool assigns(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual int value(bool &constant);
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

		void setArg(int num, Symbol *symbol);
		void removeArg(int num);

		virtual void print(const std::string &prefix);

		virtual bool assigns(Symbol *symbol);
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
		virtual int value(bool &constant);
	};

	struct Symbol {
		std::string name;
		Type *type;
		int uses;
		std::vector<Entry*> assigns;

		int *value;
		Symbol(const std::string &_name, Type *_type) : name(_name), type(_type), uses(0), value(0) {}

		void addAssign(Entry *entry);
		void removeAssign(Entry *entry);
	};

	struct Block {
		int number;
		std::vector<Block*> pred;
		std::vector<Block*> succ;

		Block(int _number) : number(_number), headEntry(Entry::TypeNone), tailEntry(Entry::TypeNone) 
		{
			headEntry.next = &tailEntry;
			tailEntry.prev = &headEntry;
		}

		void addPred(Block *block);
		void removePred(Block *block);
		void replacePred(Block *block, Block *newBlock);

		void addSucc(Block *block);
		void removeSucc(Block *block);
		void replaceSucc(Block *block, Block *newBlock);

		void appendEntry(Entry *entry) { entry->insertBefore(tail());}
		void prependEntry(Entry *entry) { entry->insertAfter(head());}

		Entry *head() { return &headEntry; }
		Entry *tail() { return &tailEntry; }

		Entry headEntry;
		Entry tailEntry;
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
		void replaceEnd(Block *block);

		std::vector<Symbol*> &symbols() { return mSymbols; }
		Symbol *newTemp(Type *type);
		Symbol *addSymbol(const std::string &name, Type *type);
		void addSymbol(Symbol *symbol);
		Symbol *findSymbol(const std::string &name);
		Block *newBlock();

		void setCurrentBlock(Block *block);

		void emit(Entry *entry);

	private:
		std::string mName;
		std::vector<Symbol*> mSymbols;
		std::vector<Block*> mBlocks;
		Block *mStart;
		Block *mEnd;

		int mNextTemp;

		Block *mCurrentBlock;
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