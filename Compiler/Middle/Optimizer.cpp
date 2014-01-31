#include "Middle/Optimizer.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"
#include "Analysis/LiveVariables.h"
#include "Analysis/Loops.h"

#include "Transform/ConstantProp.h"
#include "Transform/DeadCodeElimination.h"
#include "Transform/CopyProp.h"
#include "Transform/ThreadJumps.h"
#include "Transform/LoopInvariantCodeMotion.h"

#include "IR/Program.h"
#include "IR/Procedure.h"

#include "Util/UniqueQueue.h"
#include "Util/Timer.h"

namespace Middle {
	/*!
	 * \brief Optimize a program
	 * \param program Program to optimize
	 */
	void Optimizer::optimize(IR::Program *program)
	{
		typedef std::vector<Transform::Transform*> TransformVector;
		typedef std::map<Transform::Transform*, TransformVector> TransformToTransformVectorMap;
		TransformVector startingTransforms;
		TransformToTransformVectorMap transformMap;

		// Build the list of all transforms
		startingTransforms.push_back(Transform::CopyProp::instance());
		startingTransforms.push_back(Transform::ConstantProp::instance());
		startingTransforms.push_back(Transform::DeadCodeElimination::instance());
		startingTransforms.push_back(Transform::ThreadJumps::instance());
		startingTransforms.push_back(Transform::LoopInvariantCodeMotion::instance());

		// Transforms to run after CopyProp
		transformMap[Transform::CopyProp::instance()].push_back(Transform::DeadCodeElimination::instance());

		// Transforms to run after Constant Prop
		transformMap[Transform::ConstantProp::instance()].push_back(Transform::DeadCodeElimination::instance());

		// Transforms to run after DeadCodeElimination
		transformMap[Transform::DeadCodeElimination::instance()].push_back(Transform::ConstantProp::instance());
		transformMap[Transform::DeadCodeElimination::instance()].push_back(Transform::CopyProp::instance());

		// Optimize each procedure in turn
		for(IR::ProcedureList::iterator it = program->procedures().begin(); it != program->procedures().end(); it++) {
			IR::Procedure *procedure = *it;

			// Queue of transformations to run
			Util::UniqueQueue<Transform::Transform*> transforms;

			// Start by running each optimization pass once
			for(TransformVector::iterator itTransform = startingTransforms.begin(); itTransform != startingTransforms.end(); itTransform++) {
				Transform::Transform *transform = *itTransform;
				transforms.push(transform);
			}

			std::cout << "Optimizations:" << std::endl;

			// Run optimization passes until there are none left
			while(!transforms.empty()) {
				Transform::Transform *transform = transforms.front();
				transforms.pop();

				// Run the transform
				Util::Timer timer;
				timer.start();
				bool changed = transform->transform(procedure);
				std::cout << transform->name() << ": " << timer.stop() << "ms" << std::endl;

				if(changed) {
					// If the transform changed the IR, add its follow-up transformations to the queue
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