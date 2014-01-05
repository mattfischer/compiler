#include "TypeChecker.h"

#include "Front/SyntaxNode.h"
#include "Front/Type.h"

#include <stdio.h>
#include <stdarg.h>

namespace Front {
	bool TypeChecker::check(SyntaxNode *node)
	{
		bool result = true;

		for(unsigned int i=0; i<node->children.size(); i++) {
			SyntaxNode *child = node->children[i];
			Procedure *procedure = addProcedure(child);
			if(procedure) {
				Scope scope(*this);
				for(unsigned int j=0; j<procedure->arguments.size(); j++) {
					scope.addSymbol(procedure->arguments[j]);
				}
				result = check(child->children[2], procedure, scope);
			} else {
				result = false;
			}

			if(!result) {
				break;
			}

			child->type = TypeNone;
		}

		return result;
	}

	bool TypeChecker::check(SyntaxNode *node, Procedure *procedure, Scope &scope)
	{
		bool result = true;

		switch(node->nodeType) {
			case SyntaxNode::NodeTypeList:
			case SyntaxNode::NodeTypePrint:
				result = checkChildren(node, procedure, scope);
				node->type = TypeNone;
				break;

			case SyntaxNode::NodeTypeCall:
				{
					Procedure *procedure = findProcedure(node->children[0]->lexVal._id);
					if(procedure) {
						if(procedure->arguments.size() == node->children[1]->children.size()) {
							for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
								check(node->children[1]->children[i], procedure, scope);
								if(node->children[1]->children[i]->type != procedure->arguments[i]->type) {
									error(node, "Type mismatch on argument %i of procedure %s", i, procedure->name.c_str());
									result = false;
									break;
								}
							}
							node->type = procedure->returnType;
						} else {
							error(node, "Incorrect number of arguments to procedure %s", procedure->name.c_str());
							result = false;
						}
					} else {
						error(node, "Undeclared procedure %s", node->children[0]->lexVal._id);
						result = false;
					}
				break;
				}

			case SyntaxNode::NodeTypeVarDecl:
				result = scope.addSymbol(node->lexVal._id, node->children[1]->lexVal._id, node);
				node->type = TypeNone;
				break;

			case SyntaxNode::NodeTypeAssign:
				result = check(node->children[1], procedure, scope);
				if(result) {
					char *name = node->children[0]->lexVal._id;
					Symbol *symbol = scope.findSymbol(name);
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
				result = checkChildren(node, procedure, scope);
				if(result) {
					if(node->children[0]->type != TypeBool) {
						error(node, "Type mismatch");
						result = false;
					}

					node->type = TypeNone;
				}
				break;

			case SyntaxNode::NodeTypeCompare:
				result = checkChildren(node, procedure, scope);

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
				result = checkChildren(node, procedure, scope);

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
					Symbol *symbol = scope.findSymbol(name);
					if(symbol == NULL) {
						error(node, "Undefined variable '%s'", name);
						return false;
					}

					node->type = symbol->type;
					break;
				}

			case SyntaxNode::NodeTypeConstant:
				node->type = TypeInt;
				break;

			case SyntaxNode::NodeTypeReturn:
				check(node->children[0], procedure, scope);
				if(node->children[0]->type != procedure->returnType) {
					error(node, "Type mismatch");
					return false;
				}
				node->type = TypeNone;
				break;
		}

		return result;
	}

	bool TypeChecker::checkChildren(SyntaxNode *node, Procedure *procedure, Scope &scope)
	{
		for(unsigned int i=0; i<node->children.size(); i++) {
			bool result = check(node->children[i], procedure, scope);
			if(!result) {
				return false;
			}
		}
		return true;
	}

	TypeChecker::Scope::Scope(TypeChecker &typeChecker)
		: mTypeChecker(typeChecker)
	{
	}

	bool TypeChecker::Scope::addSymbol(const std::string &typeName, const std::string &name, SyntaxNode *node)
	{
		Type *type = Type::find(typeName);
		if(type == NULL) {
			mTypeChecker.error(node, "Error: Type '%s' not found.\n", typeName.c_str());
			return false;
		}
		
		Symbol *symbol = new Symbol(type, name);
		mSymbols.push_back(symbol);

		return true;
	}

	bool TypeChecker::Scope::addSymbol(Symbol *symbol)
	{
		Symbol *localSymbol = new Symbol(*symbol);
		mSymbols.push_back(localSymbol);

		return true;
	}

	TypeChecker::Symbol *TypeChecker::Scope::findSymbol(const std::string &name)
	{
		for(unsigned int i=0; i<mSymbols.size(); i++) {
			if(mSymbols[i]->name == name) {
				return mSymbols[i];
			}
		}

		return NULL;
	}

	TypeChecker::Procedure* TypeChecker::addProcedure(SyntaxNode *node)
	{
		Type *returnType = Type::find(node->children[0]->lexVal._id);
		if(returnType == NULL) {
			error(node, "Error: Type '%s' not found.\n", node->children[0]->lexVal._id);
			return 0;
		}

		std::vector<Symbol*> arguments;
		for(unsigned int i=0; i<node->children[1]->children.size(); i++) {
			SyntaxNode *decl = node->children[1]->children[i];
			Type *type = Type::find(decl->children[0]->lexVal._id);
			if(type == NULL) {
				error(node, "Error: Type '%s' not found.\n", decl->children[0]->lexVal._id);
				return 0;
			}
			arguments.push_back(new Symbol(type, decl->lexVal._id));
		}

		Procedure *procedure = new Procedure(returnType, node->lexVal._id, arguments);
		mProcedures.push_back(procedure);

		return procedure;
	}

	TypeChecker::Procedure *TypeChecker::findProcedure(const std::string &name)
	{
		for(unsigned int i=0; i<mProcedures.size(); i++) {
			if(mProcedures[i]->name == name) {
				return mProcedures[i];
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