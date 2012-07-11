#ifndef ANALYSIS_DOMINANCE_H
#define ANALYSIS_DOMINANCE_H

#include <vector>
#include <map>
#include <set>

#include "Analysis/FlowGraph.h"

namespace Analysis {
	class DominatorTree {
	public:
		DominatorTree(FlowGraph &flowGraph);

		const FlowGraph::BlockVector &blocks() const;
		FlowGraph::Block *idom(FlowGraph::Block *block);

	private:
		void topoSortRecurse(FlowGraph::Block *block, FlowGraph::BlockVector &blocks, FlowGraph::BlockSet &seenBlocks);
		FlowGraph::BlockVector topologicalSort(FlowGraph::BlockVector &blocks);

		FlowGraph::BlockVector mBlocks;
		std::map<FlowGraph::Block*, FlowGraph::Block*> mIDoms;
	};
}

#endif