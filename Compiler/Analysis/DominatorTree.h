#ifndef ANALYSIS_DOMINANCE_H
#define ANALYSIS_DOMINANCE_H

#include "Analysis/FlowGraph.h"

#include <vector>
#include <map>
#include <set>

namespace Analysis {
	/*!
	 * \brief Determine a tree of dominator blocks
	 *
	 * A block dominates another block if all control flow paths to the block pass through it.
	 * Dominance forms a tree--each block may dominate many blocks, which may themselves dominate
	 * other blocks, etc.
	 */
	class DominatorTree {
	public:
		DominatorTree(IR::Procedure *procedure, FlowGraph &flowGraph);

		FlowGraph &flowGraph() { return mFlowGraph; } //!< Flow graph being analyzed
		const FlowGraph::BlockVector &blocks() const;
		FlowGraph::Block *idom(FlowGraph::Block *block);
		bool dominates(FlowGraph::Block *block, FlowGraph::Block *dominator);

	private:
		void topoSortRecurse(FlowGraph::Block *block, FlowGraph::BlockVector &blocks, FlowGraph::BlockSet &seenBlocks);
		FlowGraph::BlockVector topologicalSort(FlowGraph::BlockVector &blocks);

		FlowGraph &mFlowGraph; //!< Flow graph
		FlowGraph::BlockVector mBlocks; //!< List of blocks
		std::map<FlowGraph::Block*, FlowGraph::Block*> mIDoms; //!< List of immediate dominators
	};
}

#endif