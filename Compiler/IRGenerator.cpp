#include "IRGenerator.h"

#include <sstream>

static char* names[] = {
	/* TypeLoad		*/	"load   ",
	/* TypeLoadImm	*/	"loadi  ",
	/* TypeAdd		*/	"add    ",
	/* TypeMult		*/	"mult   ",
	/* TypePrint	*/	"print  ",
	/* TypeEqual	*/	"equ    ",
	/* TypeNequal	*/	"neq    ",
	/* TypeJump		*/	"jmp    ",
	/* TypeCJump	*/	"cjmp   ",
	/* TypeNCJump	*/	"ncjmp  "
};

static void printLine(const IRLine *line)
{
	printf("%s ", names[line->type]);

	switch(line->type) {
		case IRLine::TypePrint:
			printf("%s", ((IRGenerator::List::Symbol*)line->lhs)->name.c_str());
			break;

		case IRLine::TypeLoadImm:
			printf("%s, %i", ((IRGenerator::List::Symbol*)line->lhs)->name.c_str(), line->rhs1);
			break;

		case IRLine::TypeLoad:
			printf("%s, %s", ((IRGenerator::List::Symbol*)line->lhs)->name.c_str(),
							 ((IRGenerator::List::Symbol*)line->rhs1)->name.c_str());
			break;

		case IRLine::TypeAdd:
		case IRLine::TypeMult:
		case IRLine::TypeEqual:
		case IRLine::TypeNequal:
			printf("%s, %s, %s", ((IRGenerator::List::Symbol*)line->lhs)->name.c_str(),
								 ((IRGenerator::List::Symbol*)line->rhs1)->name.c_str(),
								 ((IRGenerator::List::Symbol*)line->rhs2)->name.c_str());
			break;

		case IRLine::TypeJump:
			printf("%s", ((IRGenerator::List::Block*)line->lhs)->name.c_str());
			break;

		case IRLine::TypeCJump:
		case IRLine::TypeNCJump:
			printf("%s, %s", ((IRGenerator::List::Block*)line->lhs)->name.c_str(),
							 ((IRGenerator::List::Symbol*)line->rhs1)->name.c_str());
			break;
	}
	printf("\n");
}

void IRGenerator::List::print() const
{
	printf("Symbols:\n");
	for(int i=0; i<symbols.size(); i++) {
		printf("%s %s\n", symbols[i]->type->name.c_str(), symbols[i]->name.c_str());
	}
	printf("\n");
	printf("Lines:\n");
	for(int i=0; i<blocks.size(); i++) {
		List::Block *block = blocks[i];
		printf("%s:\n", block->name.c_str());
		for(int j=0; j<block->lines.size(); j++) {
			printf("  ");
			printLine(block->lines[j]);
		}
	}
}

IRGenerator::IRGenerator(SyntaxNode *tree)
{
	mTree = tree;
	mNextTemp = 0;
	mNextBlock = 0;
	mCurrentBlock = newBlock();
}

const IRGenerator::List &IRGenerator::generate()
{
	processNode(mTree);
	return mList;
}

void IRGenerator::processNode(SyntaxNode *node)
{
	List::Symbol *lhs, *rhs;

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

		case SyntaxNode::NodeTypeIf:
			lhs = processRValue(node->children[0]);
			if(node->numChildren == 2) {
				List::Block *oldBlock = mCurrentBlock;

				List::Block *trueBlock = newBlock();
				emit(IRLine::TypeCJump, lhs, trueBlock);
				
				setCurrentBlock(trueBlock);
				processNode(node->children[1]);

				List::Block *nextBlock = newBlock();
				setCurrentBlock(oldBlock);
				emit(IRLine::TypeJump, nextBlock);
				setCurrentBlock(trueBlock);
				emit(IRLine::TypeJump, nextBlock);

				setCurrentBlock(nextBlock);
			} else {
				List::Block *oldBlock = mCurrentBlock;

				List::Block *trueBlock = newBlock();
				emit(IRLine::TypeCJump, lhs, trueBlock);

				setCurrentBlock(trueBlock);
				processNode(node->children[1]);

				List::Block *falseBlock = newBlock();
				setCurrentBlock(falseBlock);
				processNode(node->children[2]);
				setCurrentBlock(oldBlock);
				emit(IRLine::TypeJump, falseBlock);

				List::Block *nextBlock = newBlock();
				setCurrentBlock(trueBlock);
				emit(IRLine::TypeJump, nextBlock);
				setCurrentBlock(falseBlock);
				emit(IRLine::TypeJump, nextBlock);

				setCurrentBlock(nextBlock);
			}
			break;

		case SyntaxNode::NodeTypeWhile:
			{
				List::Block *testBlock = newBlock();
				emit(IRLine::TypeJump, testBlock);
				setCurrentBlock(testBlock);
				lhs = processRValue(node->children[0]);

				List::Block *mainBlock = newBlock();
				setCurrentBlock(mainBlock);
				processNode(node->children[1]);
				emit(IRLine::TypeJump, testBlock);

				List::Block *nextBlock = newBlock();
				setCurrentBlock(testBlock);
				emit(IRLine::TypeNCJump, lhs, nextBlock);
				emit(IRLine::TypeJump, mainBlock);
				
				setCurrentBlock(nextBlock);
				break;
			}
	}
}

void IRGenerator::emit(IRLine::Type type, long lhs, long rhs1, long rhs2)
{
	IRLine *line = new IRLine(type, lhs, rhs1, rhs2);
	mCurrentBlock->lines.push_back(line);
}

IRGenerator::List::Symbol *IRGenerator::processRValue(SyntaxNode *node)
{
	List::Symbol *result;
	List::Symbol *a, *b;

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

		case SyntaxNode::NodeTypeCompare:
			result = newTemp(node->type);
			a = processRValue(node->children[0]);
			b = processRValue(node->children[1]);
			switch(node->nodeSubtype) {
				case SyntaxNode::NodeSubtypeEqual:
					emit(IRLine::TypeEqual, result, a, b);
					break;

				case SyntaxNode::NodeSubtypeNequal:
					emit(IRLine::TypeNequal, result, a, b);
					break;
			}
			break;
	}

	return result;
}

IRGenerator::List::Symbol *IRGenerator::newTemp(Type *type)
{
	std::stringstream ss;
	ss << mNextTemp++;
	std::string name = "temp" + ss.str();

	return addSymbol(name, type);
}

IRGenerator::List::Symbol *IRGenerator::addSymbol(const std::string &name, Type *type)
{
	List::Symbol *symbol = new List::Symbol(name, type);
	mList.symbols.push_back(symbol);

	return symbol;
}

IRGenerator::List::Symbol *IRGenerator::findSymbol(const std::string &name)
{
	for(int i=0; i<mList.symbols.size(); i++) {
		if(mList.symbols[i]->name == name) {
			return mList.symbols[i];
		}
	}

	return NULL;
}

IRGenerator::List::Block *IRGenerator::newBlock()
{
	std::stringstream ss;
	ss << mNextBlock++;
	std::string name = "bb" + ss.str();

	List::Block *block = new List::Block(name);
	mList.blocks.push_back(block);

	return block;
}

void IRGenerator::setCurrentBlock(List::Block *block)
{
	mCurrentBlock = block;
}