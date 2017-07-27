#ifndef IR_PROGRAM_H
#define IR_PROGRAM_H

#include "IR/Procedure.h"
#include "IR/Data.h"

#include <vector>
#include <memory>
#include <iostream>

namespace IR {
	/*!
	 * \param An entire program of IR entries
	 */
	class Program {
	public:
		Program();

		std::vector<std::unique_ptr<Procedure>> &procedures() { return mProcedures; } //!< List of all procedures
		const std::vector<std::unique_ptr<Procedure>> &procedures() const { return mProcedures; } //!< List of all procedures
		std::vector<std::unique_ptr<Data>> &data() { return mData; }

		void addProcedure(std::unique_ptr<Procedure> procedure);
		void addData(std::unique_ptr<Data> data);

		void print(std::ostream &o) const;

	private:
		std::vector<std::unique_ptr<Procedure>> mProcedures;
		std::vector<std::unique_ptr<Data>> mData;
	};
}

#endif