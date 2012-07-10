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
			Analysis::ReachingDefs defs(flowGraph);
			Analysis::UseDefs ud(procedure->blocks(), defs);

			printf("START:\n");
			ud.print();
			printf("\n");

			Transform::CopyProp::transform(procedure, ud, defs);

			printf("AFTER COPYPROP:\n");
			ud.print();
			printf("\n");

			Transform::ConstantProp::transform(procedure, ud, defs);

			printf("AFTER CONSTANTPROP:\n");
			ud.print();
			printf("\n");

			Transform::DeadCodeElimination::transform(procedure, ud);

			printf("AFTER DCE:\n");
			ud.print();
			printf("\n");
		}

		ir->print();
	}

	return 0;
}