#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "SyntaxNode.h"
#include "Type.h"

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

	std::vector<Symbol*> mSymbols;

	bool checkChildren(SyntaxNode *tree);
	bool addSymbol(const std::string &typeName, const std::string &name);
	Symbol *findSymbol(const std::string &name);
};
#endif