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
	void Program::addProcedure(std::unique_ptr<Procedure> procedure)
	{
		mProcedures.push_back(std::move(procedure));
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
		for(const std::unique_ptr<Procedure> &procedure : mProcedures) {
			o << "<" << procedure->name() << ">" << std::endl;
			procedure->print(o, "  ");
		}

		for(Data *data : mData) {
			o << "<" << data->name() << ">" << std::endl;
			data->print(o, "  ");
		}
	}
}