#include <stdio.h>

#include "Type.h"
#include "ParseNode.h"
#include "SyntaxNode.h"
#include "TypeChecker.h"
#include "Interpreter.h"
#include "IRGenerator.h"
#include "Optimizer.h"

#include "ReachingDefs.h"

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
			ReachingDefs defs(procedure->blocks());
			printf("%s:\n", procedure->name().c_str());
			defs.print();
		}
		printf("\n");

		//Optimizer opt(ir);
		//opt.optimize();
	}

	return 0;
}