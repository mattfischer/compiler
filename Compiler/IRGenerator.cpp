#include "IRGenerator.h"

#include <sstream>

void IRGenerator::List::print() const
{
	printf("Symbols:\n");
	for(int i=0; i<symbols.size(); i++) {
		printf("%s %s\n", symbols[i]->type->name.c_str(), symbols[i]->name.c_str());
	}
	printf("\n");
	printf("Lines:\n");
	for(int i=0; i<lines.size(); i++) {
		lines[i]->print();
	}
}

IRGenerator::IRGenerator(SyntaxNode *tree)
{
	mTree = tree;
	mNextTemp = 0;
}

const IRGenerator::List &IRGenerator::generate()
{
	processNode(mTree);
	return mList;
}

void IRGenerator::processNode(SyntaxNode *node)
{
	IRSymbol *lhs, *rhs;

	switch(node->nodeType) {
		case SyntaxNode::NodeTypeStatementList:
			for(int i=0; i<node->numChildren; i++) {
				processNode(node->children[i]);
			}
			break;

		case SyntaxNode::NodeTypePrintStatement:
			lhs = processRValue(node->children[0]);
			emit(IRLine::TypePrint, lhs);
			break;

		case SyntaxNode::NodeTypeVarDecl:
			addSymbol(node->children[1]->lexVal._id, Type::find(node->children[0]->lexVal._id));
			break;

		case SyntaxNode::NodeTypeAssign:
			lhs = findSymbol(node->children[0]->lexVal._id);
			if(node->children[1]->nodeType == SyntaxNode::NodeTypeConstant) {
				emit(IRLine::TypeLoadImm, lhs, node->children[1]->lexVal._int);
			} else {
				rhs = processRValue(node->children[1]);
				emit(IRLine::TypeLoad, lhs, rhs);
			}
			break;
	}
}

void IRGenerator::emit(IRLine::Type type, long lhs, long rhs1, long rhs2)
{
	IRLine *line = new IRLine(type, lhs, rhs1, rhs2);
	mList.lines.push_back(line);
}

IRSymbol *IRGenerator::processRValue(SyntaxNode *node)
{
	IRSymbol *result;
	IRSymbol *a, *b;

	switch(node->nodeType) {
		case SyntaxNode::NodeTypeConstant:
			result = newTemp(node->type);
			emit(IRLine::TypeLoadImm, result, node->lexVal._int);
			break;

		case SyntaxNode::NodeTypeId:
			result = findSymbol(node->lexVal._id);
			break;

		case SyntaxNode::NodeTypeArith:
			result = newTemp(node->type);
			a = processRValue(node->children[0]);
			b = processRValue(node->children[1]);
			switch(node->nodeSubtype) {
				case SyntaxNode::NodeSubtypeAdd:
					emit(IRLine::TypeAdd, result, a, b);
					break;

				case SyntaxNode::NodeSubtypeMultiply:
					emit(IRLine::TypeMult, result, a, b);
					break;
			}
			break;
	}

	return result;
}

IRSymbol *IRGenerator::newTemp(Type *type)
{
	std::stringstream ss;
	ss << mNextTemp++;
	std::string name = "temp" + ss.str();

	return addSymbol(name, type);
}

IRSymbol *IRGenerator::addSymbol(const std::string &name, Type *type)
{
	IRSymbol *symbol = new IRSymbol(name, type);
	mList.symbols.push_back(symbol);

	return symbol;
}

IRSymbol *IRGenerator::findSymbol(const std::string &name)
{
	for(int i=0; i<mList.symbols.size(); i++) {
		if(mList.symbols[i]->name == name) {
			return mList.symbols[i];
		}
	}

	return NULL;
}