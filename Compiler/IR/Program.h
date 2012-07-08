#ifndef IR_PROGRAM_H
#define IR_PROGRAM_H

#include <vector>

namespace IR {
	class Procedure;

	struct Program {
	public:
		Program();

		Procedure *main() { return mMain; }
		std::vector<Procedure*> procedures() { return mProcedures; }

		void print() const;

		void computeDominance();

	private:
		std::vector<Procedure*> mProcedures;
		Procedure *mMain;
	};
}

#endif