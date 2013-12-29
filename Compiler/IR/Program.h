#ifndef IR_PROGRAM_H
#define IR_PROGRAM_H

#include <list>

namespace IR {
	class Procedure;

	class Program {
	public:
		Program();

		Procedure *main() { return mMain; }
		typedef std::list<Procedure*> ProcedureList;
		ProcedureList &procedures() { return mProcedures; }

		void addProcedure(Procedure *procedure);
		Procedure *findProcedure(const std::string &name);

		void print() const;

		void computeDominance();

	private:
		ProcedureList mProcedures;
		Procedure *mMain;
	};
}

#endif