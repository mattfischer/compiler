#ifndef IR_PROGRAM_H
#define IR_PROGRAM_H

#include "IR/Procedure.h"
#include "IR/Data.h"

#include <list>
#include <iostream>

namespace IR {
	/*!
	 * \param An entire program of IR entries
	 */
	class Program {
	public:
		Program();

		Procedure *main() { return mMain; } //!< Main procedure
		ProcedureList &procedures() { return mProcedures; } //!< List of all procedures
		DataList &data() { return mData; }

		void addProcedure(Procedure *procedure);
		Procedure *findProcedure(const std::string &name);

		void addData(Data *data);

		void print(std::ostream &o) const;

	private:
		ProcedureList mProcedures;
		DataList mData;
		Procedure *mMain;
	};
}

#endif