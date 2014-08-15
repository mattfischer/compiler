#include "Front/Interpreter.h"

#include "Front/Node.h"
#include "Front/Type.h"
#include "Front/Types.h"

#include <iostream>

namespace Front {
	Interpreter::Interpreter(Node *tree)
	{
		mTree = tree;
	}

	void Interpreter::run()
	{
		for(unsigned int i=0; i<mTree->children.size(); i++) {
			Node *procedure = mTree->children[i];
			addProcedure(procedure->lexVal.s, procedure->children[1]);
		}

		Procedure *proc = findProcedure("main");
		evaluateStatement(proc->body);
	}

	void Interpreter::evaluateStatement(Node *node)
	{
		switch(node->nodeType) {
			case Node::Type::List:
				for(unsigned int i=0; i<node->children.size(); i++) {
					evaluateStatement(node->children[i]);
				}
				break;

			case Node::Type::VarDecl:
				{
					Type *type = 0; // FIXME: Type::find(node->children[0]->lexVal.s);
					addVariable(node->lexVal.s, type);
					break;
				}

			case Node::Type::Assign:
				{
					Variable *variable = findVariable(node->children[0]->lexVal.s);
					evaluateExpression(node->children[1]);

					pop(variable->data, node->children[1]->type->valueSize);
					break;
				}

			case Node::Type::If:
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

			case Node::Type::While:
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

	void Interpreter::evaluateExpression(Node *node)
	{
		switch(node->nodeType) {
			case Node::Type::Constant:
				push(&node->lexVal.i, sizeof(int));
				break;

			case Node::Type::Id:
				{
					Variable *variable = findVariable(node->lexVal.s);
					push(variable->data, node->type->valueSize);
					break;
				}

			case Node::Type::Compare:
				{
					int a, b, c;

					evaluateExpression(node->children[0]);
					pop(&a, sizeof(a));

					evaluateExpression(node->children[1]);
					pop(&b, sizeof(b));

					switch(node->nodeSubtype) {
						case Node::Subtype::Equal:
							c = (a == b) ? 1 : 0;
							break;

						case Node::Subtype::Nequal:
							c = (a != b) ? 1 : 0;
							break;
					}

					push(&c, sizeof(c));
					break;
				}

			case Node::Type::Arith:
				{
					int a, b, c;
					evaluateExpression(node->children[0]);
					pop(&a, sizeof(a));

					evaluateExpression(node->children[1]);
					pop(&b, sizeof(b));

					switch(node->nodeSubtype) {
						case Node::Subtype::Add:
							c = a + b;
							break;

						case Node::Subtype::Multiply:
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
		variable->data = new char[type->allocSize];

		mVariables.push_back(variable);
	}

	Interpreter::Variable *Interpreter::findVariable(const std::string &name)
	{
		for(unsigned int i=0; i<mVariables.size(); i++) {
			if(mVariables[i]->name == name) {
				return mVariables[i];
			}
		}

		return 0;
	}

	void Interpreter::addProcedure(const std::string &name, Node *body)
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

		return 0;
	}
}
