#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <string>
#include <vector>

namespace IR {
	class Program;
	class Procedure;
}

class Optimizer {
public:
	class Pass {
	public:
		Pass(const std::string &name) : mName(name) {}

		const std::string &name() const { return mName; }

		virtual bool procedures() { return false; }

		virtual bool optimizeProcedure(IR::Procedure *proc) { return false; }

	private:
		std::string mName;
	};

	Optimizer(IR::Program *ir);

	void optimize();

	IR::Program *ir() { return mIR; }

private:
	IR::Program *mIR;
	std::vector<Pass*> mPreSSAPasses;
	std::vector<Pass*> mPostSSAPasses;

	void initPasses();
	void repeatPasses(std::vector<Pass*> &passes);
	bool optimizeProcedures(Pass *pass);
};

#endif