#ifndef ANALYSIS_BLOCK_SORT_H
#define ANALYSIS_BLOCK_SORT_H

#include <vector>
#include <set>
#include <map>

#include "Analysis/FlowGraph.h"

namespace Analysis {
	class BlockSort {
	public:
		BlockSort(FlowGraph &flowGraph);

		const FlowGraph::BlockVector &sorted() const;
		int position(FlowGraph::Block *block) const;

	private:
		void sortRecurse(FlowGraph::Block *block, FlowGraph::BlockVector &blocks, FlowGraph::BlockSet &seenBlocks);

		FlowGraph::BlockVector mSorted;
		typedef std::map<FlowGraph::Block*, int> OrderMap;
		OrderMap mOrder;
	};
}
#endif