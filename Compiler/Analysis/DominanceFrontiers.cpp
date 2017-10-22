#include "Analysis/DominanceFrontiers.h"

#include "Analysis/DominatorTree.h"

namespace Analysis {
	static std::set<const FlowGraph::Block*> emptyBlockSet; //!< Empty block set, used when block lookup fails

	/*!
	 * \brief Constructor
	 * \param tree Dominator tree to analyze
	 */
	DominanceFrontiers::DominanceFrontiers(const DominatorTree &tree)
	{
		for(const FlowGraph::Block *block : tree.blocks()) {
			if(block->pred.size() < 2)
				continue;

			for(const FlowGraph::Block *runner : block->pred) {
				while(runner != tree.idom(block)) {
					mFrontiers[runner].insert(block);
					runner = tree.idom(runner);
				}
			}
		}
	}

	/*!
	 * \brief Return dominance frontiers for a given block
	 * \param block Block to analyze
	 * \return Dominance frontiers for block
	 */
	const std::set<const FlowGraph::Block*> &DominanceFrontiers::frontiers(const FlowGraph::Block *block) const
	{
		auto it = mFrontiers.find(block);
		if(it != mFrontiers.end()) {
			return it->second;
		} else {
			return emptyBlockSet;
		}
	}
}