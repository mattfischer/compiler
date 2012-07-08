#ifndef IR_ENTRY_H
#define IR_ENTRY_H

#include <string>

namespace IR {
	class Block;
	class Symbol;

	class Entry {
	public:
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
		virtual Symbol *assignSymbol() { return 0; }
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
		virtual Symbol *assignSymbol();
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
		virtual Symbol *assignSymbol();
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
		virtual Symbol *assignSymbol();
	};
}

#endif
