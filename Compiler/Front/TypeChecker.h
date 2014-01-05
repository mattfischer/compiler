#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <string>
#include <vector>

namespace Front {
	struct Node;
	class Type;

	class TypeChecker
	{
	public:
		bool check(Node *tree);

	private:
		struct Symbol {
			Type *type;
			std::string name;

			Symbol(Type *_type, const std::string &_name) : type(_type), name(_name) {}
			Symbol(const Symbol &other) : type(other.type), name(other.name) {}
		};

		class Scope {
		public:
			Scope(TypeChecker &typeChecker);

			bool addSymbol(const std::string &typeName, const std::string &name, Node *tree);
			bool addSymbol(Symbol *symbol);
			Symbol *findSymbol(const std::string &name);

		private:
			TypeChecker &mTypeChecker;
			std::vector<Symbol*> mSymbols;
		};

		struct Procedure {
			Type *returnType;
			std::string name;
			std::vector<Symbol*> arguments;

			Procedure(Type *_returnType, const std::string &_name, const std::vector<Symbol*> _arguments) : returnType(_returnType), name(_name), arguments(_arguments) {}
		};

		std::vector<Procedure*> mProcedures;

		bool check(Node *tree, Procedure *procedure, Scope &scope);
		bool checkChildren(Node *tree, Procedure *procedure, Scope &scope);

		Procedure *addProcedure(Node *tree);
		Procedure *findProcedure(const std::string &name);

		void error(Node *node, char *fmt, ...);
	};
}
#endif