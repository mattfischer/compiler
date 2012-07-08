#include <stdio.h>

#include "Type.h"
#include "ParseNode.h"
#include "SyntaxNode.h"
#include "TypeChecker.h"
#include "Interpreter.h"
#include "IRGenerator.h"
#include "Optimizer.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"

extern "C" ParseNode *parseLang(const char *filename);

int main(int arg, char *argv[])
{
	Type::init();

	ParseNode *parseTree = parseLang("input.lang");
	SyntaxNode *syntaxTree = SyntaxNode::fromParseTree(parseTree);
	TypeChecker typeChecker;
	bool result = typeChecker.check(syntaxTree);

	if(result) {
		/*Interpreter interpreter(syntaxTree);
		interpreter.run();*/

		IRGenerator generator(syntaxTree);
		IR *ir = generator.generate();

		printf("REACHING DEFS:\n");
	
		for(int i=0; i<ir->procedures().size(); i++) {
			IR::Procedure *procedure = ir->procedures()[i];
			Analysis::ReachingDefs defs(procedure->blocks());
			Analysis::UseDefs ud(procedure->blocks(), defs);
			ud.print();
		}
		printf("\n");

		//Optimizer opt(ir);
		//opt.optimize();
	}

	return 0;
}