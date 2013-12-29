#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <vector>

namespace Front {
	struct SyntaxNode;
	class Type;

	class Interpreter
	{
	public:
		Interpreter(Front::SyntaxNode *tree);

		void run();

	private:
		Front::SyntaxNode *mTree;

		void evaluateStatementList(Front::SyntaxNode *node);
		void evaluateStatement(Front::SyntaxNode *node);
		void evaluateExpression(Front::SyntaxNode *node);

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
			Front::SyntaxNode *body;
		};

		std::vector<Procedure*> mProcedures;

		void addProcedure(const std::string &name, Front::SyntaxNode *body);
		Procedure *findProcedure(const std::string &name);
	};
}

#endif