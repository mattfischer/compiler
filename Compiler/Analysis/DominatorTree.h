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
		DominatorTree(const IR::Procedure &procedure, FlowGraph &flowGraph);

		const std::vector<FlowGraph::Block*> &blocks() const;
		FlowGraph::Block *idom(FlowGraph::Block *block);
		bool dominates(FlowGraph::Block *block, FlowGraph::Block *dominator);

	private:
		void topoSortRecurse(FlowGraph::Block *block, std::vector<FlowGraph::Block*> &blocks, std::set<FlowGraph::Block*> &seenBlocks);
		std::vector<FlowGraph::Block*> topologicalSort(std::vector<FlowGraph::Block*> &blocks);

		FlowGraph *mFlowGraph; //!< Flow graph
		std::vector<FlowGraph::Block*> mBlocks; //!< List of blocks
		std::map<FlowGraph::Block*, FlowGraph::Block*> mIDoms; //!< List of immediate dominators
	};
}

#endif