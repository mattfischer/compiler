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
		enum class Type {
			None, //!< Used for list internals
			Move, //!< Move one variable to another
			Add, //!< Add two symbols together
			Subtract, //!< Subtract two symbols
			Mult, //!< Multiply two symbols together
			Divide, //!< Divide two symbols
			Modulo, //!< Take modulus of two symbols
			Equal, //!< Test two symbols for equality
			Nequal, //!< Test two symbols for inequality
			LessThan, //!< Test two symbols for less-than
			LessThanE, //!< Test two symbols for less-than-equal
			GreaterThan, //!< Test two symbols for greater-than
			GreaterThanE, //!< Test two symbols for greater-than-equal
			And, //!< Boolean AND
			Or, //!< Boolean OR
			Label, //!< A label, used as a jump target
			Jump, //!< Jump to a specified label
			CJump, //!< Jump to one of two labels based on a symbol's value
			Phi, //!< Phi function for SSA
			Call, //!< Call a procedure
			CallIndirect, //!< Call a procedure via an address in a symbol
			LoadRet, //!< Load a value from the return register
			StoreRet, //!< Store a value in the return register
			LoadArg, //!< Load a value from an argument register
			StoreArg, //!< Store a value in an argument register
			LoadStack, //!< Load a value from a stack location
			StoreStack, //!< Store a value to a stack location
			LoadAddress, //!< Load the address of a symbol
			Prologue, //!< Function prologue
			Epilogue, //!< Function epilogue
			New, //!< Allocate memory
			StoreMem, //!< Store to memory
			LoadMem, //!< Load from memory
			LoadString, //!< Load a string constant
			FunctionAddr, //!< Function address
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
		virtual const Symbol *assign() const { return 0; }

		/*!
		 * \brief Check if an entry uses a given symbol
		 * \param symbol Symbol to check for
		 * \return True if entry uses symbol
		 */
		virtual bool uses(const Symbol *symbol) const { return false; }

		/*!
		 * \brief Replace the assignment of a symbol with another symbol
		 * \param symbol Original symbol
		 * \param newSymbol New symbol
		 */
		virtual void replaceAssign(const Symbol *symbol, const Symbol *newSymbol) {}

		/*!
		 * \brief Replace all uses of a symbol with another symbol
		 * \param symbol Original symbol
		 * \param newSymbol New symbol
		 */
		virtual void replaceUse(const Symbol *symbol, const Symbol *newSymbol) {}
	};

	/*!
	 * \brief Three-address entry: one assignment symbol and two arguments
	 */
	struct EntryThreeAddr : public Entry {
		const Symbol *lhs; //!< Left-hand side
		const Symbol *rhs1; //!< Right-hand side 1
		const Symbol *rhs2; //!< Right-hand side 2
		int    imm; //!< Immediate value

		/*!
		 * \brief Constructor
		 * \param _type Entry type
		 * \param _lhs Left-hand side, or 0
		 * \param _rhs1 Right-hand side 1, or 0
		 * \param _rhs2 Right-hand side 2, or 0
		 */
		EntryThreeAddr(Type _type, const Symbol *_lhs = 0, const Symbol *_rhs1 = 0, const Symbol *_rhs2 = 0, int _imm = 0);
		virtual ~EntryThreeAddr();

		virtual void print(std::ostream &o) const;

		virtual const Symbol *assign() const;
		virtual bool uses(const Symbol *symbol) const;
		virtual void replaceAssign(const Symbol *symbol, const Symbol *newSymbol);
		virtual void replaceUse(const Symbol *symbol, const Symbol *newSymbol);
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
		const Symbol *pred; //!< Conditional predicate
		EntryLabel *trueTarget; //!< Label to jump to if true
		EntryLabel *falseTarget; //!< Label to jump to if false

		/*!
		 * \brief Constructor
		 * \param _pred Conditional predicate
		 * \param _trueTarget Label to jump to if true
		 * \param _falseTarget Label to jump to if false
		 */
		EntryCJump(const Symbol *_pred, EntryLabel *_trueTarget, EntryLabel *_falseTarget);
		virtual ~EntryCJump();

		virtual void print(std::ostream &o) const;

		virtual bool uses(const Symbol *symbol) const;
		virtual void replaceUse(const Symbol *symbol, const Symbol *newSymbol);
	};

	/*!
	 * \brief SSA Phi function entry
	 */
	struct EntryPhi : public Entry {
		const Symbol *base; //!< Symbol that phi function derives from
		const Symbol *lhs; //!< Left-hand side
		int numArgs; //!< Number of arguments
		const Symbol **args; //!< Arguments to function

		/*!
		 * \brief Constructor
		 * \param _base Symbol that phi function derives from
		 * \param _lhs Left-hand side
		 * \param _numArgs Number of arguments
		 */
		EntryPhi(const Symbol *_base, const Symbol *_lhs, int _numArgs);
		virtual ~EntryPhi();

		/*!
		 * \brief Set a given argument
		 * \param num Argument number
		 * \param symbol Symbol to assign to argument
		 */
		void setArg(int num, const Symbol *symbol);

		/*!
		 * \brief Remove an argument
		 * \param num Argument number
		 */
		void removeArg(int num);

		virtual void print(std::ostream &o) const;

		virtual const Symbol *assign() const;
		virtual bool uses(const Symbol *symbol) const;
		virtual void replaceAssign(const Symbol *symbol, const Symbol *newSymbol);
		virtual void replaceUse(const Symbol *symbol, const Symbol *newSymbol);
	};

	/*!
	 * \brief Procedure call entry
	 */
	struct EntryCall : public Entry {
		std::string target; //!< Procedure to call

		/*!
		 * \brief Constructor
		 * \param _target Procedure to call
		 */
		EntryCall(Type _type, const std::string &_target);

		virtual void print(std::ostream &o) const;
	};

	struct EntryString : public Entry {
		const Symbol *lhs;
		std::string string;

		EntryString(Type _type, const Symbol *_lhs, const std::string &_string);

		virtual void print(std::ostream &o) const;

		virtual const Symbol *assign() const;
		virtual void replaceAssign(const Symbol *symbol, const Symbol *newSymbol);
	};

	std::ostream &operator<<(std::ostream &o, const Entry &entry);
}

#endif
