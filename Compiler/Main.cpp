#include <stdio.h>

#include "Front/Parser.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"

#include "IR/Program.h"
#include "IR/Procedure.h"

#include "Transform/ConstantProp.h"

int main(int arg, char *argv[])
{
	Front::Parser parser("input.lang");
	IR::Program *ir = parser.ir();
	if(ir) {
		for(unsigned int i=0; i<ir->procedures().size(); i++) {
			IR::Procedure *procedure = ir->procedures()[i];
			Analysis::ReachingDefs defs(procedure->blocks());
			Analysis::UseDefs ud(procedure->blocks(), defs);

			printf("START:\n");
			ud.print();
			printf("\n");

			Transform::ConstantProp::transform(procedure, ud);

			printf("AFTER CONSTANTPROP:\n");
			ud.print();
			printf("\n");
		}
	}

	return 0;
}