#include <stdio.h>

#include "Front/Parser.h"

#include "Analysis/Analysis.h"

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
			Analysis::Analysis analysis(procedure);

			printf("Start:\n");
			analysis.useDefs().print();
			printf("\n");

			printf("Reaching Defs:\n");
			analysis.reachingDefs().print();
			printf("\n");

			Transform::CopyProp::instance().transform(procedure, analysis);

			printf("After CopyProp:\n");
			analysis.useDefs().print();
			printf("\n");

			Transform::ConstantProp::instance().transform(procedure, analysis);

			printf("After ConstantProp:\n");
			analysis.useDefs().print();
			printf("\n");

			Transform::DeadCodeElimination::instance().transform(procedure, analysis);

			printf("After DeadCodeElimination:\n");
			analysis.useDefs().print();
			printf("\n");

			Transform::ConstantProp::instance().transform(procedure, analysis);

			printf("After ConstantProp:\n");
			analysis.useDefs().print();
			printf("\n");

			Transform::DeadCodeElimination::instance().transform(procedure, analysis);

			printf("After DeadCodeElimination:\n");
			analysis.useDefs().print();
			printf("\n");
		}
	}

	return 0;
}