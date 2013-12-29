#include "TypeChecker.h"

#include "Front/SyntaxNode.h"
#include "Front/Type.h"

#include <stdio.h>
#include <stdarg.h>

namespace Front {
	bool TypeChecker::check(SyntaxNode *node)
	{
		bool result = true;

		switch(node->nodeType) {
			case SyntaxNode::NodeTypeProcedureList:
			case SyntaxNode::NodeTypeStatementList:
			case SyntaxNode::NodeTypePrintStatement:
				result = checkChildren(node);
				node->type = TypeNone;
				break;

			case SyntaxNode::NodeTypeProcedure:
				result = check(node->children[2]);
				break;

			case SyntaxNode::NodeTypeVarDecl:
				result = addSymbol(node->children[0]->lexVal._id, node->children[1]->lexVal._id, node);
				node->type = TypeNone;
				break;

			case SyntaxNode::NodeTypeAssign:
				result = check(node->children[1]);
				if(result) {
					char *name = node->children[0]->lexVal._id;
					Symbol *symbol = findSymbol(name);
					if(symbol) {
						if(symbol->type != node->children[1]->type) {
							error(node, "Type mismatch");
							result = false;
						}
					} else {
						error(node, "Undeclared variable '%s'", name);
						result = false;
					}

					node->type = TypeNone;
				}
				break;

			case SyntaxNode::NodeTypeIf:
			case SyntaxNode::NodeTypeWhile:
				result = checkChildren(node);
				if(result) {
					if(node->children[0]->type != TypeBool) {
						error(node, "Type mismatch");
						result = false;
					}

					node->type = TypeNone;
				}
				break;

			case SyntaxNode::NodeTypeCompare:
				result = checkChildren(node);

				if(result) {
					if(node->children[0]->type != TypeInt ||
						node->children[1]->type != TypeInt) {
							error(node, "Type mismatch");
							result = false;
					}
					node->type = TypeBool;
				}
				break;

			case SyntaxNode::NodeTypeArith:
				result = checkChildren(node);

				if(result) {
					if(node->children[0]->type != TypeInt ||
						node->children[1]->type != TypeInt) {
							error(node, "Type mismatch");
							result = false;
					}
					node->type = TypeInt;
				}
				break;

			case SyntaxNode::NodeTypeId:
				{
					const char *name = node->lexVal._id;
					Symbol *symbol = findSymbol(name);
					if(symbol == NULL) {
						error(node, "Undefined variable '%s'", name);
						return false;
					}

					node->type = symbol->type;
					break;
				}
		}

		return result;
	}

	bool TypeChecker::checkChildren(SyntaxNode *node)
	{
		for(int i=0; i<node->numChildren; i++) {
			bool result = check(node->children[i]);
			if(!result) {
				return false;
			}
		}
		return true;
	}

	bool TypeChecker::addSymbol(const std::string &typeName, const std::string &name, SyntaxNode *node)
	{
		Type *type = Type::find(typeName);
		if(type == NULL) {
			error(node, "Error: Type '%s' not found.\n", typeName.c_str());
			return false;
		}
		
		Symbol *symbol = new Symbol(type, name);
		mSymbols.push_back(symbol);

		return true;
	}

	TypeChecker::Symbol *TypeChecker::findSymbol(const std::string &name)
	{
		for(unsigned int i=0; i<mSymbols.size(); i++) {
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
}