#include "Analysis/DominanceFrontiers.h"

#include "Analysis/DominatorTree.h"

namespace Analysis {
	static FlowGraph::BlockSet emptyBlockSet; //!< Empty block set, used when block lookup fails

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
	const FlowGraph::BlockSet &DominanceFrontiers::frontiers(FlowGraph::Block *block) const
	{
		std::map<FlowGraph::Block*, FlowGraph::BlockSet>::const_iterator it = mFrontiers.find(block);
		if(it != mFrontiers.end()) {
			return it->second;
		} else {
			return emptyBlockSet;
		}
	}
}