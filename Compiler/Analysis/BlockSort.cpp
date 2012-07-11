#include "BlockSort.h"

#include <algorithm>

namespace Analysis {
	void BlockSort::sortRecurse(FlowGraph::Block *block, FlowGraph::BlockVector &blocks, FlowGraph::BlockSet &seenBlocks)
	{
		if(seenBlocks.find(block) != seenBlocks.end()) {
			return;
		}

		seenBlocks.insert(block);

		for(FlowGraph::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
			FlowGraph::Block *succ = *it;
			sortRecurse(succ, blocks, seenBlocks);
		}

		blocks.push_back(block);
	}

	BlockSort::BlockSort(FlowGraph &flowGraph)
	{
		FlowGraph::BlockSet seenBlocks;

		for(FlowGraph::BlockSet::iterator it = flowGraph.blocks().begin(); it != flowGraph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
			sortRecurse(block, mSorted, seenBlocks);
		}

		std::reverse(mSorted.begin(), mSorted.end());
	}

	const FlowGraph::BlockVector &BlockSort::sorted() const
	{
		return mSorted;
	}

	int BlockSort::position(FlowGraph::Block *block) const
	{
		OrderMap::const_iterator it = mOrder.find(block);
		if(it != mOrder.end()) {
			return it->second;
		} else {
			return -1;
		}
	}
}