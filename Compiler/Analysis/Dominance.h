#ifndef ANALYSIS_DOMINANCE_H
#define ANALYSIS_DOMINANCE_H

#include <vector>
#include <map>
#include <set>

#include "Analysis/FlowGraph.h"

namespace Analysis {
	class DominatorTree {
	public:
		typedef std::vector<FlowGraph::Block*> BlockVector;

		DominatorTree(FlowGraph &flowGraph);

		const BlockVector &blocks() const;
		FlowGraph::Block *idom(FlowGraph::Block *block);

	private:
		typedef std::set<FlowGraph::Block*> BlockSet;
		void topoSortRecurse(FlowGraph::Block *block, BlockVector &blocks, BlockSet &seenBlocks);
		BlockVector topologicalSort(BlockVector &blocks);

		BlockVector mBlocks;
		std::map<FlowGraph::Block*, FlowGraph::Block*> mIDoms;
	};

	class DominanceFrontiers {
	public:
		DominanceFrontiers(DominatorTree &tree);

		typedef std::set<FlowGraph::Block*> BlockSet;
		typedef std::vector<FlowGraph::Block*> BlockVector;
		const BlockSet &frontiers(FlowGraph::Block *block) const;

	private:
		std::map<FlowGraph::Block*, BlockSet> mFrontiers;
	};
}

#endif