#include <stdio.h>

#include "Front/Parser.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"
#include "Analysis/FlowGraph.h"

#include "IR/Program.h"
#include "IR/Procedure.h"

#include "Transform/ConstantProp.h"
#include "Transform/DeadCodeElimination.h"
#include "Transform/CopyProp.h"

int main(int arg, char *argv[])
{
	Front::Parser parser("input.lang");
	IR::Program *ir = parser.ir();
	if(ir) {
		for(IR::Program::ProcedureList::iterator it = ir->procedures().begin(); it != ir->procedures().end(); it++) {
			IR::Procedure *procedure = *it;
			Analysis::FlowGraph flowGraph(procedure);
			Analysis::ReachingDefs defs(procedure, flowGraph);
			Analysis::UseDefs ud(procedure, defs);

			printf("Start:\n");
			ud.print();
			printf("\n");

			printf("Reaching Defs:\n");
			defs.print();
			printf("\n");

			Transform::CopyProp::transform(procedure, ud, defs);

			printf("After CopyProp:\n");
			ud.print();
			printf("\n");

			Transform::ConstantProp::transform(procedure, ud, defs, flowGraph);

			printf("After ConstantProp:\n");
			ud.print();
			printf("\n");

			Transform::DeadCodeElimination::transform(procedure, ud, defs, flowGraph);

			printf("After DeadCodeElimination:\n");
			ud.print();
			printf("\n");

			Transform::ConstantProp::transform(procedure, ud, defs, flowGraph);

			printf("After ConstantProp:\n");
			ud.print();
			printf("\n");

			Transform::DeadCodeElimination::transform(procedure, ud, defs, flowGraph);

			printf("After DeadCodeElimination:\n");
			ud.print();
			printf("\n");
		}
	}

	return 0;
}