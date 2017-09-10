#include "Analysis/DominanceFrontiers.h"

#include "Analysis/DominatorTree.h"

namespace Analysis {
	static std::set<FlowGraph::Block*> emptyBlockSet; //!< Empty block set, used when block lookup fails

	/*!
	 * \brief Constructor
	 * \param tree Dominator tree to analyze
	 */
	DominanceFrontiers::DominanceFrontiers(DominatorTree &tree)
	{
		for(FlowGraph::Block *block : tree.blocks()) {
			if(block->pred.size() < 2)
				continue;

			for(FlowGraph::Block *runner : block->pred) {
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
	const std::set<FlowGraph::Block*> &DominanceFrontiers::frontiers(FlowGraph::Block *block) const
	{
		std::map<FlowGraph::Block*, std::set<FlowGraph::Block*>>::const_iterator it = mFrontiers.find(block);
		if(it != mFrontiers.end()) {
			return it->second;
		} else {
			return emptyBlockSet;
		}
	}
}