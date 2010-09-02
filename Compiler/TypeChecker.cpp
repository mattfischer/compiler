#include "TypeChecker.h"

#include <stdio.h>
#include <stdarg.h>

bool TypeChecker::check(SyntaxNode *tree)
{
	bool result = true;

	switch(tree->nodeType) {
		case SyntaxNode::NodeTypeStatementList:
		case SyntaxNode::NodeTypePrintStatement:
			result = checkChildren(tree);
			tree->type = TypeNone;
			break;

		case SyntaxNode::NodeTypeVarDecl:
			result = addSymbol(tree->children[0]->lexVal._id, tree->children[1]->lexVal._id, tree);
			break;

		case SyntaxNode::NodeTypeAssign:
			result = check(tree->children[1]);
			if(result) {
				Symbol *symbol = findSymbol(tree->children[0]->lexVal._id);
				if(symbol->type != tree->children[1]->type) {
					error(tree, "Type mismatch");
					return false;
				}
			}
			break;

		case SyntaxNode::NodeTypeEqual:
			result = checkChildren(tree);

			if(result) {
				if(tree->children[0]->type != TypeInt ||
					tree->children[1]->type != TypeInt) {
						error(tree, "Type mismatch");
						return false;
				}
				tree->type = TypeBool;
			}
			break;

		case SyntaxNode::NodeTypeAdd:
		case SyntaxNode::NodeTypeMultiply:
			result = checkChildren(tree);

			if(result) {
				if(tree->children[0]->type != TypeInt ||
					tree->children[1]->type != TypeInt) {
						error(tree, "Type mismatch");
						return false;
				}
				tree->type = TypeInt;
			}
			break;

		case SyntaxNode::NodeTypeId:
			{
				const char *name = tree->lexVal._id;
				Symbol *symbol = findSymbol(name);
				if(symbol == NULL) {
					error(tree, "Undefined variable '%s'", name);
					return false;
				}

				tree->type = symbol->type;
				break;
			}
	}

	return result;
}

bool TypeChecker::checkChildren(SyntaxNode *tree)
{
	for(int i=0; i<tree->numChildren; i++) {
		bool result = check(tree->children[i]);
		if(!result) {
			return false;
		}
	}
	return true;
}

bool TypeChecker::addSymbol(const std::string &typeName, const std::string &name, SyntaxNode *tree)
{
	Type *type = Type::find(typeName);
	if(type == NULL) {
		error(tree, "Error: Type '%s' not found.\n", typeName.c_str());
		return false;
	}
	
	Symbol *symbol = new Symbol(type, name);
	mSymbols.push_back(symbol);

	return true;
}

TypeChecker::Symbol *TypeChecker::findSymbol(const std::string &name)
{
	for(int i=0; i<mSymbols.size(); i++) {
		if(mSymbols[i]->name == name) {
			return mSymbols[i];
		}
	}

	return NULL;
}

void TypeChecker::error(SyntaxNode *node, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("Error, line %i: ", node->line);
	vprintf(fmt, args);
	printf("\n");

	va_end(args);
}