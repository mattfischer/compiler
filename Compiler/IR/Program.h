#ifndef IR_PROGRAM_H
#define IR_PROGRAM_H

#include "IR/Procedure.h"

#include <list>

namespace IR {
	/*!
	 * \param An entire program of IR entries
	 */
	class Program {
	public:
		Program();

		Procedure *main() { return mMain; } //!< Main procedure
		ProcedureList &procedures() { return mProcedures; } //!< List of all procedures

		void addProcedure(Procedure *procedure);
		Procedure *findProcedure(const std::string &name);

		void print() const;

	private:
		ProcedureList mProcedures;
		Procedure *mMain;
	};
}

#endif