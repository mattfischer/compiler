#ifndef ANALYSIS_DOMINANCE_FRONTIERS_H
#define ANALYSIS_DOMINANCE_FRONTIERS_H

#include <set>
#include <vector>

#include "Analysis/FlowGraph.h"

namespace Analysis {
	class DominatorTree;

	class DominanceFrontiers {
	public:
		DominanceFrontiers(DominatorTree &tree);

		const FlowGraph::BlockSet &frontiers(FlowGraph::Block *block) const;

	private:
		std::map<FlowGraph::Block*, FlowGraph::BlockSet> mFrontiers;
	};
}
#endif