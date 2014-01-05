#include "Front/Interpreter.h"

#include "Front/SyntaxNode.h"
#include "Front/Type.h"

#include <stdio.h>

namespace Front {
	Interpreter::Interpreter(SyntaxNode *tree)
	{
		mTree = tree;
	}

	void Interpreter::run()
	{
		for(int i=0; i<mTree->children.size(); i++) {
			SyntaxNode *procedure = mTree->children[i];
			addProcedure(procedure->lexVal._id, procedure->children[1]);
		}

		Procedure *proc = findProcedure("main");
		evaluateStatement(proc->body);
	}

	void Interpreter::evaluateStatement(SyntaxNode *node)
	{
		switch(node->nodeType) {
			case SyntaxNode::NodeTypeList:
				for(int i=0; i<node->children.size(); i++) {
					evaluateStatement(node->children[i]);
				}
				break;

			case SyntaxNode::NodeTypePrint:
				{
					int result;

					evaluateExpression(node->children[0]);
					pop(&result, sizeof(result));

					Type *type = node->children[0]->type;
					if(type == TypeInt)
						printf("%i\n", result);
					else if(type == TypeBool)
						printf("%s\n", (result == 1) ? "true" : "false");

					break;
				}
			case SyntaxNode::NodeTypeVarDecl:
				{
					Type *type = Type::find(node->children[0]->lexVal._id);
					addVariable(node->lexVal._id, type);
					break;
				}

			case SyntaxNode::NodeTypeAssign:
				{
					Variable *variable = findVariable(node->children[0]->lexVal._id);
					evaluateExpression(node->children[1]);

					pop(variable->data, node->children[1]->type->size);
					break;
				}

			case SyntaxNode::NodeTypeIf:
				{
					int a;
					evaluateExpression(node->children[0]);
					pop(&a, sizeof(a));

					if(a == 1) {
						evaluateStatement(node->children[1]);
					} else if(node->children.size() == 3) {
						evaluateStatement(node->children[2]);
					}
					break;
				}

			case SyntaxNode::NodeTypeWhile:
				for(;;) {
					int a;
					evaluateExpression(node->children[0]);
					pop(&a, sizeof(a));

					if(a == 0) {
						break;
					}

					evaluateStatement(node->children[1]);
				}
				break;
		}
	}

	void Interpreter::evaluateExpression(SyntaxNode *node)
	{
		switch(node->nodeType) {
			case SyntaxNode::NodeTypeConstant:
				push(&node->lexVal._int, sizeof(int));
				break;

			case SyntaxNode::NodeTypeId:
				{
					Variable *variable = findVariable(node->lexVal._id);
					push(variable->data, node->type->size);
					break;
				}

			case SyntaxNode::NodeTypeCompare:
				{
					int a, b, c;

					evaluateExpression(node->children[0]);
					pop(&a, sizeof(a));

					evaluateExpression(node->children[1]);
					pop(&b, sizeof(b));

					switch(node->nodeSubtype) {
						case SyntaxNode::NodeSubtypeEqual:
							c = (a == b) ? 1 : 0;
							break;

						case SyntaxNode::NodeSubtypeNequal:
							c = (a != b) ? 1 : 0;
							break;
					}

					push(&c, sizeof(c));
					break;
				}

			case SyntaxNode::NodeTypeArith:
				{
					int a, b, c;
					evaluateExpression(node->children[0]);
					pop(&a, sizeof(a));

					evaluateExpression(node->children[1]);
					pop(&b, sizeof(b));

					switch(node->nodeSubtype) {
						case SyntaxNode::NodeSubtypeAdd:
							c = a + b;
							break;

						case SyntaxNode::NodeSubtypeMultiply:
							c = a * b;
							break;
					}

					push(&c, sizeof(c));
					break;
				}
		}
	}

	void Interpreter::push(void *data, int size)
	{
		char *c = (char*)data;

		for(int i=0; i<size; i++) {
			mStack.push_back(c[i]);
		}
	}

	void Interpreter::pop(void *data, int size)
	{
		char *c = (char*)data;

		for(int i=size-1; i>=0; i--) {
			c[i] = mStack.back();
			mStack.pop_back();
		}
	}

	void Interpreter::addVariable(const std::string &name, Type *type)
	{
		Variable *variable = new Variable;

		variable->name = name;
		variable->data = new char[type->size];

		mVariables.push_back(variable);
	}

	Interpreter::Variable *Interpreter::findVariable(const std::string &name)
	{
		for(unsigned int i=0; i<mVariables.size(); i++) {
			if(mVariables[i]->name == name) {
				return mVariables[i];
			}
		}

		return NULL;
	}

	void Interpreter::addProcedure(const std::string &name, SyntaxNode *body)
	{
		Procedure *procedure = new Procedure;

		procedure->name = name;
		procedure->body = body;

		mProcedures.push_back(procedure);
	}

	Interpreter::Procedure *Interpreter::findProcedure(const std::string &name)
	{
		for(unsigned int i=0; i<mProcedures.size(); i++) {
			if(mProcedures[i]->name == name) {
				return mProcedures[i];
			}
		}

		return NULL;
	}
}
