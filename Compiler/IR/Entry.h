#ifndef IR_ENTRY_H
#define IR_ENTRY_H

#include "IR/Symbol.h"

#include <string>
#include <iostream>
#include <set>

/*!
 * \brief Intermediate Representation
 *
 * Used for transitioning from the front-end to the back-end, and for optimization passes
 */
namespace IR {
	class Procedure;

	/*!
	 * \brief A single instruction in an IR program
	 */
	class Entry {
	public:
		/*!
		 * \brief Type of entry
		 */
		enum Type {
			TypeNone, //!< Used for list internals
			TypeMove, //!< Move one variable to another
			TypeAdd, //!< Add two symbols together
			TypeSubtract, //!< Subtract two symbols
			TypeMult, //!< Multiply two symbols together
			TypePrintInt, //!< Print a symbol
			TypePrintString, //!< Print a string
			TypeEqual, //!< Test two symbols for equality
			TypeNequal, //!< Test two symbols for inequality
			TypeLessThan, //!< Test two symbols for less-than
			TypeLessThanE, //!< Test two symbols for less-than-equal
			TypeGreaterThan, //!< Test two symbols for greater-than
			TypeGreaterThanE, //!< Test two symbols for greater-than-equal
			TypeAnd, //!< Boolean AND
			TypeOr, //!< Boolean OR
			TypeLabel, //!< A label, used as a jump target
			TypeJump, //!< Jump to a specified label
			TypeCJump, //!< Jump to one of two labels based on a symbol's value
			TypePhi, //!< Phi function for SSA
			TypeCall, //!< Call a procedure
			TypeLoadRet, //!< Load a value from the return register
			TypeStoreRet, //!< Store a value in the return register
			TypeLoadArg, //!< Load a value from an argument register
			TypeStoreArg, //!< Store a value in an argument register
			TypeLoadStack, //!< Load a value from a stack location
			TypeStoreStack, //!< Store a value to a stack location
			TypePrologue, //!< Function prologue
			TypeEpilogue, //!< Function epilogue
			TypeNew, //!< Allocate memory
			TypeStoreMem, //!< Store to memory
			TypeLoadMem, //!< Load from memory
			TypeLoadString //!< Load a string constant
		};

		Type type; //!< Entry type

		Entry *prev; //!< Linked list prev pointer
		Entry *next; //!< Linked list next pointer

		/*!
		 * \brief Base class constructor
		 * \param _type Entry type
		 */
		Entry(Type _type) : type(_type), prev(0), next(0) {}

		/*!
		 * \brief Virtual destructor
		 */
		virtual ~Entry() {}

		/*!
		 * \brief Print entry to a stream
		 * \param o Stream to print to
		 */
		virtual void print(std::ostream &o) const {}

		/*!
		 * \brief Symbol that entry assigns to
		 * \return Assignment symbol, or 0 if none
		 */
		virtual Symbol *assign() { return 0; }

		/*!
		 * \brief Check if an entry uses a given symbol
		 * \param symbol Symbol to check for
		 * \return True if entry uses symbol
		 */
		virtual bool uses(Symbol *symbol) { return false; }

		/*!
		 * \brief Replace the assignment of a symbol with another symbol
		 * \param symbol Original symbol
		 * \param newSymbol New symbol
		 */
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol) {}

		/*!
		 * \brief Replace all uses of a symbol with another symbol
		 * \param symbol Original symbol
		 * \param newSymbol New symbol
		 */
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol) {}
	};

	/*!
	 * \brief Three-address entry: one assignment symbol and two arguments
	 */
	struct EntryThreeAddr : public Entry {
		Symbol *lhs; //!< Left-hand side
		Symbol *rhs1; //!< Right-hand side 1
		Symbol *rhs2; //!< Right-hand side 2
		int    imm; //!< Immediate value

		/*!
		 * \brief Constructor
		 * \param _type Entry type
		 * \param _lhs Left-hand side, or 0
		 * \param _rhs1 Right-hand side 1, or 0
		 * \param _rhs2 Right-hand side 2, or 0
		 */
		EntryThreeAddr(Type _type, Symbol *_lhs = 0, Symbol *_rhs1 = 0, Symbol *_rhs2 = 0, int _imm = 0);
		virtual ~EntryThreeAddr();

		virtual void print(std::ostream &o) const;

		virtual Symbol *assign();
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	/*!
	 * \brief Label entry
	 */
	struct EntryLabel : public Entry {
		std::string name; //!< Label name

		/*!
		 * \brief Constructor
		 * \param _name Label name
		 */
		EntryLabel(const std::string &_name);

		virtual void print(std::ostream &o) const;
	};

	/*!
	 * \brief Jump entry
	 */
	struct EntryJump : public Entry {
		EntryLabel *target; //!< Target label

		/*!
		 * \brief Constructor
		 * \param _target Target label
		 */
		EntryJump(EntryLabel *_target);

		virtual void print(std::ostream &o) const;
	};

	/*!
	 * \brief Conditional jump entry
	 */
	struct EntryCJump : public Entry {
		Symbol *pred; //!< Conditional predicate
		EntryLabel *trueTarget; //!< Label to jump to if true
		EntryLabel *falseTarget; //!< Label to jump to if false

		/*!
		 * \brief Constructor
		 * \param _pred Conditional predicate
		 * \param _trueTarget Label to jump to if true
		 * \param _falseTarget Label to jump to if false
		 */
		EntryCJump(Symbol *_pred, EntryLabel *_trueTarget, EntryLabel *_falseTarget);
		virtual ~EntryCJump();

		virtual void print(std::ostream &o) const;

		virtual bool uses(Symbol *symbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	/*!
	 * \brief SSA Phi function entry
	 */
	struct EntryPhi : public Entry {
		Symbol *base; //!< Symbol that phi function derives from
		Symbol *lhs; //!< Left-hand side
		int numArgs; //!< Number of arguments
		Symbol **args; //!< Arguments to function

		/*!
		 * \brief Constructor
		 * \param _base Symbol that phi function derives from
		 * \param _lhs Left-hand side
		 * \param _numArgs Number of arguments
		 */
		EntryPhi(Symbol *_base, Symbol *_lhs, int _numArgs);
		virtual ~EntryPhi();

		/*!
		 * \brief Set a given argument
		 * \param num Argument number
		 * \param symbol Symbol to assign to argument
		 */
		void setArg(int num, Symbol *symbol);

		/*!
		 * \brief Remove an argument
		 * \param num Argument number
		 */
		void removeArg(int num);

		virtual void print(std::ostream &o) const;

		virtual Symbol *assign();
		virtual bool uses(Symbol *symbol);
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
		virtual void replaceUse(Symbol *symbol, Symbol *newSymbol);
	};

	/*!
	 * \brief Procedure call entry
	 */
	struct EntryCall : public Entry {
		Procedure *target; //!< Procedure to call

		/*!
		 * \brief Constructor
		 * \param _target Procedure to call
		 */
		EntryCall(Procedure *_target);

		virtual void print(std::ostream &o) const;
	};

	struct EntryString : public Entry {
		Symbol *lhs;
		std::string string;

		EntryString(Type _type, Symbol *_lhs, const std::string &_string);

		virtual void print(std::ostream &o) const;

		virtual Symbol *assign();
		virtual void replaceAssign(Symbol *symbol, Symbol *newSymbol);
	};

	std::ostream &operator<<(std::ostream &o, const Entry &entry);

	typedef std::set<Entry*> EntrySet; //!< Convenience typedef
}

#endif
