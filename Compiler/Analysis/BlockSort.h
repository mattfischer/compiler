#ifndef ANALYSIS_BLOCK_SORT_H
#define ANALYSIS_BLOCK_SORT_H

#include <vector>
#include <set>
#include <map>

#include "Analysis/FlowGraph.h"

namespace Analysis {
	class BlockSort {
	public:
		typedef std::vector<FlowGraph::Block*> BlockVector;
		BlockSort(FlowGraph &flowGraph);

		const BlockVector &sorted() const;
		int position(FlowGraph::Block *block) const;

	private:
		typedef std::set<FlowGraph::Block*> BlockSet;
		void sortRecurse(FlowGraph::Block *block, BlockVector &blocks, BlockSet &seenBlocks);

		BlockVector mSorted;
		typedef std::map<FlowGraph::Block*, int> OrderMap;
		OrderMap mOrder;
	};
}
#endif