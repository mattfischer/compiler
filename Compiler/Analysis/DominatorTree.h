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
		DominatorTree(const IR::Procedure &procedure, const FlowGraph &flowGraph);

		const std::vector<const FlowGraph::Block*> &blocks() const;
		const FlowGraph::Block *idom(const FlowGraph::Block *block) const;
		bool dominates(const FlowGraph::Block *block, const FlowGraph::Block *dominator) const;

	private:
		void topoSortRecurse(const FlowGraph::Block *block, std::vector<const FlowGraph::Block*> &blocks, std::set<const FlowGraph::Block*> &seenBlocks);
		std::vector<const FlowGraph::Block*> topologicalSort(std::vector<FlowGraph::Block*> &blocks);

		std::vector<const FlowGraph::Block*> mBlocks; //!< List of blocks
		std::map<const FlowGraph::Block*, const FlowGraph::Block*> mIDoms; //!< List of immediate dominators
	};
}

#endif