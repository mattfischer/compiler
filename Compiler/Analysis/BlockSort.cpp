#include "BlockSort.h"

#include <algorithm>

namespace Analysis {
	/*!
	 * \brief Recursive sort procedure
	 * \param block Block to insert into list
	 * \param blocks Block list under construction
	 * \param seenBlocks Blocks already encountered
	 */
	void BlockSort::sortRecurse(FlowGraph::Block *block, FlowGraph::BlockVector &blocks, FlowGraph::BlockSet &seenBlocks)
	{
		// Skip block if already seen
		if(seenBlocks.find(block) != seenBlocks.end()) {
			return;
		}

		seenBlocks.insert(block);

		// Add all successors of this block into the list
		for(FlowGraph::Block *succ : block->succ) {
			sortRecurse(succ, blocks, seenBlocks);
		}

		// Finally, add this block to the list
		blocks.push_back(block);
	}

	/*!
	 * \brief Constructor
	 * \param flowGraph Control flow graph to sort
	 */
	BlockSort::BlockSort(FlowGraph *flowGraph)
	{
		FlowGraph::BlockSet seenBlocks;

		// Add each block into the list in sequence
		for(FlowGraph::Block *block : flowGraph->blocks()) {
			sortRecurse(block, mSorted, seenBlocks);
		}

		// Now reverse the list so that earlier block occur earlier in the list
		std::reverse(mSorted.begin(), mSorted.end());
		for(unsigned int i=0; i<mSorted.size(); i++) {
			mOrder[mSorted[i]] = i;
		}
	}

	/*!
	 * \brief Return a sorted list of blocks
	 * \return List of blocks
	 */
	const FlowGraph::BlockVector &BlockSort::sorted() const
	{
		return mSorted;
	}

	/*!
	 * \brief Return a block's position in the list
	 * \param block Block to examine
	 * \return Block's index in list, or -1 if not in list
	 */
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