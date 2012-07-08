#include "Front/Parser.h"

#include "IR/Program.h"

#include "Front/SyntaxNode.h"
#include "Front/TypeChecker.h"
#include "Front/IRGenerator.h"

#include "ParseNode.h"

extern "C" ParseNode *parseLang(const char *filename);

namespace Front {
	Parser::Parser(const std::string &filename)
	{
		ParseNode *parseTree = parseLang("input.lang");
		SyntaxNode *syntaxTree = SyntaxNode::fromParseTree(parseTree);
		TypeChecker typeChecker;
		bool result = typeChecker.check(syntaxTree);

		if(result) {
			IRGenerator generator(syntaxTree);
			mIR = generator.generate();
		} else {
			mIR = 0;
		}
	}

	IR::Program *Parser::ir()
	{
		return mIR;
	}
}