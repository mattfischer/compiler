#include "Middle/Optimizer.h"

#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"
#include "Analysis/LiveVariables.h"
#include "Analysis/Loops.h"
#include "Analysis/Analysis.h"

#include "Transform/ConstantProp.h"
#include "Transform/DeadCodeElimination.h"
#include "Transform/CopyProp.h"
#include "Transform/ThreadJumps.h"
#include "Transform/LoopInvariantCodeMotion.h"
#include "Transform/CommonSubexpressionElimination.h"

#include "IR/Program.h"
#include "IR/Procedure.h"

#include "Util/UniqueQueue.h"
#include "Util/Timer.h"
#include "Util/Log.h"

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
		startingTransforms.push_back(Transform::CommonSubexpressionElimination::instance());

		// Transforms to run after CopyProp
		transformMap[Transform::CopyProp::instance()].push_back(Transform::DeadCodeElimination::instance());

		// Transforms to run after Constant Prop
		transformMap[Transform::ConstantProp::instance()].push_back(Transform::DeadCodeElimination::instance());

		// Transforms to run after DeadCodeElimination
		transformMap[Transform::DeadCodeElimination::instance()].push_back(Transform::ConstantProp::instance());
		transformMap[Transform::DeadCodeElimination::instance()].push_back(Transform::CopyProp::instance());

		// Transforms to run after CommonSubexpressionElimination
		transformMap[Transform::CommonSubexpressionElimination::instance()].push_back(Transform::CopyProp::instance());

		// Optimize each procedure in turn
		for(std::unique_ptr<IR::Procedure> &procedure : program->procedures()) {
			Analysis::Analysis analysis(*procedure);

			// Queue of transformations to run
			Util::UniqueQueue<Transform::Transform*> transforms;

			// Start by running each optimization pass once
			for(Transform::Transform *transform : startingTransforms) {
				transforms.push(transform);
			}

			Util::log("opt") << "Optimizations (" << procedure->name() << "):" << std::endl;

			// Run optimization passes until there are none left
			while(!transforms.empty()) {
				Transform::Transform *transform = transforms.front();
				transforms.pop();

				// Run the transform
				Util::Timer timer;
				timer.start();
				bool changed = transform->transform(*procedure, analysis);
				Util::log("opt.time") << transform->name() << ": " << timer.stop() << "ms" << std::endl;

				if(changed) {
					procedure->print(Util::log("opt.ir"));
					Util::log("opt.ir") << std::endl;

					// If the transform changed the IR, add its follow-up transformations to the queue
					TransformVector &newTransforms = transformMap[transform];
					for(Transform::Transform *newTransform : newTransforms) {
						transforms.push(newTransform);
					}
				}
			}

			Util::log("opt") << std::endl;
		}
	}
}