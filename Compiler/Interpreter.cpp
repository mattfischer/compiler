#include "Interpreter.h"

#include <stdio.h>

Interpreter::Interpreter(SyntaxNode *tree)
{
	mTree = tree;
}

void Interpreter::run()
{
	evaluateStatementList(mTree);
}

void Interpreter::evaluateStatementList(SyntaxNode *node)
{
	for(int i=0; i<node->numChildren; i++) {
		evaluateStatement(node->children[i]);
	}
}

void Interpreter::evaluateStatement(SyntaxNode *node)
{
	switch(node->nodeType) {
		case SyntaxNode::NodeTypePrintStatement:
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
				addVariable(node->children[1]->lexVal._id, type);
				break;
			}

		case SyntaxNode::NodeTypeAssign:
			{
				Variable *variable = findVariable(node->children[0]->lexVal._id);
				evaluateExpression(node->children[1]);

				pop(variable->data, node->children[1]->type->size);
				break;
			}
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

		case SyntaxNode::NodeTypeEqual:
			{
				int a, b, c;

				evaluateExpression(node->children[0]);
				pop(&a, sizeof(a));

				evaluateExpression(node->children[1]);
				pop(&b, sizeof(b));

				c = (a == b) ? 1 : 0;
				push(&c, sizeof(c));
				break;
			}

		case SyntaxNode::NodeTypeAdd:
			{
				int a, b, c;
				evaluateExpression(node->children[0]);
				pop(&a, sizeof(a));

				evaluateExpression(node->children[1]);
				pop(&b, sizeof(b));

				c = a + b;
				push(&c, sizeof(c));
				break;
			}

		case SyntaxNode::NodeTypeMultiply:
			{
				int a, b, c;
				evaluateExpression(node->children[0]);
				pop(&a, sizeof(a));

				evaluateExpression(node->children[1]);
				pop(&b, sizeof(b));

				c = a * b;
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
	for(int i=0; i<mVariables.size(); i++) {
		if(mVariables[i]->name == name) {
			return mVariables[i];
		}
	}

	return NULL;
}