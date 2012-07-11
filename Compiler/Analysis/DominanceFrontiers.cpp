#include "Analysis/DominanceFrontiers.h"

#include "Analysis/DominatorTree.h"

namespace Analysis {
	static FlowGraph::BlockSet emptyBlockSet;

	DominanceFrontiers::DominanceFrontiers(DominatorTree &tree)
	{
		for(unsigned int i=0; i<tree.blocks().size(); i++) {
			FlowGraph::Block *block = tree.blocks()[i];

			if(block->pred.size() < 2)
				continue;

			for(FlowGraph::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
				FlowGraph::Block *runner = *it;
				while(runner != tree.idom(block)) {
					mFrontiers[runner].insert(block);
					runner = tree.idom(runner);
				}
			}
		}
	}

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