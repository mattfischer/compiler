#include "IR/Program.h"

#include "IR/Procedure.h"

namespace IR {
	/*!
	 * \brief Constructor
	 */
	Program::Program()
	{
	}

	/*!
	 * \brief Add a procedure to the program
	 * \param procedure Procedure to add
	 */
	void Program::addProcedure(Procedure *procedure)
	{
		mProcedures.push_back(procedure);
	}

	/*!
	 * \brief Find a procedure by name
	 * \param name Name of procedure
	 * \return Procedure if found, or 0
	 */
	Procedure *Program::findProcedure(const std::string &name)
	{
		for(Procedure* procedure : mProcedures) {
			if(procedure->name() == name) {
				return procedure;
			}
		}

		return 0;
	}

	/*!
	 * \brief Add a data section to the program
	 * \param data Data to add
	 */
	void Program::addData(Data *data)
	{
		mData.push_back(data);
	}

	/*!
	 * \brief Print program
	 */
	void Program::print(std::ostream &o) const
	{
		for(Procedure *procedure : mProcedures) {
			o << "<" << procedure->name() << ">" << std::endl;
			procedure->print(o, "  ");
		}

		for(Data *data : mData) {
			o << "<" << data->name() << ">" << std::endl;
			data->print(o, "  ");
		}
	}
}