#include "IR/Program.h"

#include "IR/Procedure.h"

namespace IR {
	/*!
	 * \brief Constructor
	 */
	Program::Program()
	{
		mMain = 0;
	}

	/*!
	 * \brief Add a procedure to the program
	 * \param procedure Procedure to add
	 */
	void Program::addProcedure(Procedure *procedure)
	{
		mProcedures.push_back(procedure);
		if(procedure->name() == "main") {
			mMain = procedure;
		}
	}

	/*!
	 * \brief Find a procedure by name
	 * \param name Name of procedure
	 * \return Procedure if found, or 0
	 */
	Procedure *Program::findProcedure(const std::string &name)
	{
		for(ProcedureList::iterator it = mProcedures.begin(); it != mProcedures.end(); it++) {
			Procedure* procedure = *it;
			if(procedure->name() == name) {
				return procedure;
			}
		}

		return 0;
	}

	/*!
	 * \brief Print program
	 */
	void Program::print() const
	{
		for(ProcedureList::const_iterator it = mProcedures.begin(); it != mProcedures.end(); it++) {
			Procedure *procedure = *it;
			std::cout << "<" << procedure->name() << ">" << std::endl;
			procedure->print("  ");
		}
	}
}