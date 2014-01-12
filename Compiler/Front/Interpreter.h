#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Front/Node.h"
#include "Front/Type.h"

#include <vector>

namespace Front {
	class Interpreter
	{
	public:
		Interpreter(Front::Node *tree);

		void run();

	private:
		Front::Node *mTree;

		void evaluateStatementList(Front::Node *node);
		void evaluateStatement(Front::Node *node);
		void evaluateExpression(Front::Node *node);

		void push(void *data, int size);
		void pop(void *data, int size);

		std::vector<char> mStack;

		struct Variable {
			std::string name;
			char *data;
		};

		std::vector<Variable*> mVariables;

		void addVariable(const std::string &name, Front::Type *type);
		Variable *findVariable(const std::string &name);

		struct Procedure {
			std::string name;
			Front::Node *body;
		};

		std::vector<Procedure*> mProcedures;

		void addProcedure(const std::string &name, Front::Node *body);
		Procedure *findProcedure(const std::string &name);
	};
}

#endif