#include "Middle/Optimizer.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"
#include "Analysis/LiveVariables.h"
#include "Analysis/Loops.h"

#include "Transform/ConstantProp.h"
#include "Transform/DeadCodeElimination.h"
#include "Transform/CopyProp.h"
#include "Transform/ThreadJumps.h"

#include "IR/Program.h"
#include "IR/Procedure.h"

#include "Util/UniqueQueue.h"

namespace Middle {
	void Optimizer::optimize(IR::Program *program)
	{
		typedef std::vector<Transform::Transform*> TransformVector;
		typedef std::map<Transform::Transform*, TransformVector> TransformToTransformVectorMap;
		TransformVector startingTransforms;
		TransformToTransformVectorMap transformMap;

		startingTransforms.push_back(Transform::CopyProp::instance());
		startingTransforms.push_back(Transform::ConstantProp::instance());
		startingTransforms.push_back(Transform::DeadCodeElimination::instance());
		startingTransforms.push_back(Transform::ThreadJumps::instance());

		transformMap[Transform::CopyProp::instance()].push_back(Transform::DeadCodeElimination::instance());
		transformMap[Transform::ConstantProp::instance()].push_back(Transform::DeadCodeElimination::instance());

		transformMap[Transform::DeadCodeElimination::instance()].push_back(Transform::ConstantProp::instance());
		transformMap[Transform::DeadCodeElimination::instance()].push_back(Transform::CopyProp::instance());

		for(IR::Program::ProcedureList::iterator it = program->procedures().begin(); it != program->procedures().end(); it++) {
			IR::Procedure *procedure = *it;

			printf("Start:\n");
			procedure->print();
			printf("\n");

			Analysis::ReachingDefs reachingDefs(procedure);
			Analysis::UseDefs useDefs(procedure);
			Analysis::LiveVariables liveVariables(procedure);

			printf("Reaching Definitions:\n");
			reachingDefs.print();
			printf("\n");

			printf("Use-def chains:\n");
			useDefs.print();
			printf("\n");

			printf("Live Variables:\n");
			liveVariables.print();
			printf("\n");

			Util::UniqueQueue<Transform::Transform*> transforms;

			for(TransformVector::iterator itTransform = startingTransforms.begin(); itTransform != startingTransforms.end(); itTransform++) {
				Transform::Transform *transform = *itTransform;
				transforms.push(transform);
			}

			while(!transforms.empty()) {
				Transform::Transform *transform = transforms.front();
				transforms.pop();

				bool changed = transform->transform(procedure);

				printf("After %s:\n", transform->name().c_str());
				procedure->print();
				printf("\n");

				if(changed) {
					TransformVector &newTransforms = transformMap[transform];
					for(TransformVector::iterator itTransform = newTransforms.begin(); itTransform != newTransforms.end(); itTransform++) {
						Transform::Transform *newTransform = *itTransform;
						transforms.push(newTransform);
					}
				}
			}
		}
	}
}