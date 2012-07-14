#include "Middle/Optimizer.h"

#include "Analysis/Analysis.h"

#include "Transform/ConstantProp.h"
#include "Transform/DeadCodeElimination.h"
#include "Transform/CopyProp.h"

#include "IR/Program.h"

#include <queue>

namespace Middle {
	void Optimizer::optimize(IR::Program *program)
	{
		for(IR::Program::ProcedureList::iterator it = program->procedures().begin(); it != program->procedures().end(); it++) {
			IR::Procedure *procedure = *it;
			Analysis::Analysis analysis(procedure);

			printf("Start:\n");
			analysis.useDefs().print();
			printf("\n");

			typedef std::queue<Transform::Transform*> TransformQueue;
			TransformQueue transforms;
			transforms.push(Transform::CopyProp::instance());
			transforms.push(Transform::ConstantProp::instance());
			transforms.push(Transform::DeadCodeElimination::instance());
			transforms.push(Transform::ConstantProp::instance());
			transforms.push(Transform::DeadCodeElimination::instance());

			while(!transforms.empty()) {
				Transform::Transform *transform = transforms.front();
				transforms.pop();

				transform->transform(procedure, analysis);

				printf("After %s:\n", transform->name().c_str());
				analysis.useDefs().print();
				printf("\n");
			}
		}
	}
}