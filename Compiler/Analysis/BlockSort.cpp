#include "BlockSort.h"

#include <algorithm>

namespace Analysis {
	void BlockSort::sortRecurse(IR::Block *block, BlockVector &blocks, BlockSet &seenBlocks)
	{
		if(seenBlocks.find(block) != seenBlocks.end()) {
			return;
		}

		seenBlocks.insert(block);

		for(unsigned int i=0; i<block->succ.size(); i++) {
			sortRecurse(block->succ[i], blocks, seenBlocks);
		}

		blocks.push_back(block);
	}

	BlockSort::BlockSort(const BlockVector &blocks)
	{
		BlockSet seenBlocks;

		for(unsigned int i=0; i<blocks.size(); i++) {
			sortRecurse(blocks[i], mSorted, seenBlocks);
		}

		std::reverse(mSorted.begin(), mSorted.end());
	}

	const BlockSort::BlockVector &BlockSort::sorted() const
	{
		return mSorted;
	}

	int BlockSort::position(IR::Block *block) const
	{
		OrderMap::const_iterator it = mOrder.find(block);
		if(it != mOrder.end()) {
			return it->second;
		} else {
			return -1;
		}
	}
}