#include "Front/Parser.h"

#include "IR/Program.h"

#include "Front/SyntaxNode.h"
#include "Front/TypeChecker.h"
#include "Front/IRGenerator.h"
#include "Front/Tokenizer.h"
#include "Front/HandParser.h"

#include "ParseNode.h"

extern "C" ParseNode *parseLang(const char *filename);

namespace Front {
	IR::Program *Parser::parse(const std::string &filename)
	{
		Tokenizer tokenizer("input.lang");
		HandParser handParser(tokenizer);

		SyntaxNode *syntaxTree = handParser.parse();
		TypeChecker typeChecker;
		bool result = typeChecker.check(syntaxTree);

		IR::Program *program = 0;
		if(result) {
			IRGenerator generator;
			program = generator.generate(syntaxTree);
		}

		return program;
	}
}