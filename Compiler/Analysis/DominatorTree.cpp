#include "Analysis/DominatorTree.h"

#include "Analysis/BlockSort.h"

#include <algorithm>

namespace Analysis {
	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 * \param flowGraph Flow graph for procedure
	 */
	DominatorTree::DominatorTree(const IR::Procedure &procedure, const FlowGraph &flowGraph)
	{
		// Assign an order to the blocks
		BlockSort sort(flowGraph);
		mBlocks = sort.sorted();

		mIDoms[mBlocks[0]] = mBlocks[0];

		bool changed;
		do {
			changed = false;
			for(unsigned int i=1; i<mBlocks.size(); i++) {
				const FlowGraph::Block *block = mBlocks[i];
				const FlowGraph::Block *newDom = 0;
				for(const FlowGraph::Block *pred : block->pred) {
					if(mIDoms[pred] == 0) {
						continue;
					}

					if(newDom) {
						const FlowGraph::Block *a = pred;
						const FlowGraph::Block *b = newDom;
						while(a != b) {
							while(sort.position(a) > sort.position(b))
								a = mIDoms[a];
							while(sort.position(b) > sort.position(a))
								b = mIDoms[b];
						}
						newDom = a;
					} else {
						newDom = pred;
					}
				}

				if(newDom != mIDoms[block]) {
					mIDoms[block] = newDom;
					changed = true;
				}
			}
		} while(changed);
	}

	/*!
	 * \brief Return the immediate dominator of a block--the nearest block which dominates it
	 * \param block Block to search for
	 * \return Immediate dominator for block
	 */
	const FlowGraph::Block *DominatorTree::idom(const FlowGraph::Block *block) const
	{
		auto it = mIDoms.find(block);
		if (it == mIDoms.end()) {
			return 0;
		}
		else {
			return it->second;
		}
	}

	/*!
	 * \brief Return list of all blocks
	 * \return List of blocks
	 */
	const std::vector<const FlowGraph::Block*> &DominatorTree::blocks() const
	{
		return mBlocks;
	}

	/*!
	 * \brief Determine if a given block is dominated by another
	 * \param block Block to check for dominance of
	 * \param dominator Block which may dominate the given block
	 * \return True if dominator dominates block, otherwise false
	 */
	bool DominatorTree::dominates(const FlowGraph::Block *block, const FlowGraph::Block *dominator) const
	{
		const FlowGraph::Block *cursor = block;
		const FlowGraph::Block *id = idom(cursor);

		// Iterate upwards through the dominator tree, looking for dominance
		while(id != cursor) {
			cursor = id;
			id = idom(cursor);
			if(cursor == dominator) {
				return true;
			}
		}

		return false;
	}
}