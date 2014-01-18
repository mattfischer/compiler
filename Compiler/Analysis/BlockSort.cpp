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
		for(FlowGraph::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
			FlowGraph::Block *succ = *it;
			sortRecurse(succ, blocks, seenBlocks);
		}

		// Finally, add this block to the list
		blocks.push_back(block);
	}

	/*!
	 * \brief Constructor
	 * \param flowGraph Control flow graph to sort
	 */
	BlockSort::BlockSort(FlowGraph &flowGraph)
	{
		FlowGraph::BlockSet seenBlocks;

		// Add each block into the list in sequence
		for(FlowGraph::BlockSet::iterator it = flowGraph.blocks().begin(); it != flowGraph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
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