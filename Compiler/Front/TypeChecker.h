#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <string>
#include <vector>

namespace Front {
	struct SyntaxNode;
	class Type;

	class TypeChecker
	{
	public:
		bool check(SyntaxNode *tree);

	private:
		struct Symbol {
			Type *type;
			std::string name;

			Symbol(Type *_type, const std::string &_name) : type(_type), name(_name) {}
		};

		class Scope {
		public:
			Scope(TypeChecker &typeChecker);

			bool addSymbol(const std::string &typeName, const std::string &name, SyntaxNode *tree);
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

		bool check(SyntaxNode *tree, Procedure *procedure, Scope &scope);
		bool checkChildren(SyntaxNode *tree, Procedure *procedure, Scope &scope);

		Procedure *addProcedure(SyntaxNode *tree);
		Procedure *findProcedure(const std::string &name);

		void error(SyntaxNode *node, char *fmt, ...);
	};
}
#endif