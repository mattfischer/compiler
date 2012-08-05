#ifndef ANALYSIS_DOMINANCE_H
#define ANALYSIS_DOMINANCE_H

#include <vector>
#include <map>
#include <set>

#include "Analysis/FlowGraph.h"

namespace Analysis {
	class DominatorTree {
	public:
		DominatorTree(IR::Procedure *procedure, FlowGraph &flowGraph);

		FlowGraph &flowGraph() { return mFlowGraph; }
		const FlowGraph::BlockVector &blocks() const;
		FlowGraph::Block *idom(FlowGraph::Block *block);
		bool dominates(FlowGraph::Block *block, FlowGraph::Block *dominator);

	private:
		void topoSortRecurse(FlowGraph::Block *block, FlowGraph::BlockVector &blocks, FlowGraph::BlockSet &seenBlocks);
		FlowGraph::BlockVector topologicalSort(FlowGraph::BlockVector &blocks);

		FlowGraph &mFlowGraph;
		FlowGraph::BlockVector mBlocks;
		std::map<FlowGraph::Block*, FlowGraph::Block*> mIDoms;
	};
}

#endif