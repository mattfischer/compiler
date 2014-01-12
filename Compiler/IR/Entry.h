#ifndef IR_ENTRY_H
#define IR_ENTRY_H

#include "IR/Symbol.h"

#include <string>
#include <iostream>

namespace IR {
	class Procedure;

	class Entry {
	public:
		enum Type {
			TypeNone,
			TypeMove,
			TypeLoadImm,
			TypeAdd,
			TypeAddImm,
			TypeMult,
			TypePrint,
			TypeEqual,
			TypeNequal,
			TypeLabel,
			TypeJump,
			TypeCJump,
			TypeNCJump,
			TypePhi,
			TypeCall,
			TypeLoadRet,
			TypeStoreRet,
			TypeLoadArg,
			TypeStoreArg,
			TypeLoadStack,
			TypeStoreStack,
			TypePrologue,
			TypeEpilogue
		};

		Type type;

		Entry *prev;
		Entry *next;

		Entry(Type _type) : type(_type), prev(0), next(0) {}
		virtual ~Entry() {}

		virtual void print(std::ostream &o) const {}

		virtual Symbol *assign() { return 0; }
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

		virtual void print(std::ostream &o) const;

		virtual Symbol *assign();
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	struct EntryOneAddrImm : public Entry {
		Symbol *lhs;
		int imm;

		EntryOneAddrImm(Type _type, Symbol *_lhs, int _imm);
		virtual ~EntryOneAddrImm();

		virtual void print(std::ostream &o) const;

		virtual Symbol *assign();
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
	};

	struct EntryTwoAddrImm : public Entry {
		Symbol *lhs;
		Symbol *rhs;
		int imm;

		EntryTwoAddrImm(Type _type, Symbol *_lhs, Symbol *_rhs, int _imm);
		virtual ~EntryTwoAddrImm();

		virtual void print(std::ostream &o) const;

		virtual Symbol *assign();
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	struct EntryLabel : public Entry {
		std::string name;

		EntryLabel(const std::string &_name);

		virtual void print(std::ostream &o) const;
	};

	struct EntryJump : public Entry {
		EntryLabel *target;

		EntryJump(EntryLabel *_target);

		virtual void print(std::ostream &o) const;
	};

	struct EntryCJump : public Entry {
		Symbol *pred;
		EntryLabel *trueTarget;
		EntryLabel *falseTarget;

		EntryCJump(Symbol *_pred, EntryLabel *_trueTarget, EntryLabel *_falseTarget);
		virtual ~EntryCJump();

		virtual void print(std::ostream &o) const;

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

		virtual void print(std::ostream &o) const;

		virtual Symbol *assign();
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	struct EntryCall : public Entry {
		Procedure *target;

		EntryCall(Procedure *_target);

		virtual void print(std::ostream &o) const;
	};

	std::ostream &operator<<(std::ostream &o, const Entry &entry);
}

#endif
