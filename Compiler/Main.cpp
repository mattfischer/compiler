#include <stdio.h>

#include "Front/Parser.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"

#include "IR/Program.h"
#include "IR/Procedure.h"

int main(int arg, char *argv[])
{
	Front::Parser parser("input.lang");
	IR::Program *ir = parser.ir();
	if(ir) {
		printf("REACHING DEFS:\n");
	
		for(unsigned int i=0; i<ir->procedures().size(); i++) {
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