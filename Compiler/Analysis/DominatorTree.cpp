#include "Analysis/DominatorTree.h"

#include "Analysis/BlockSort.h"
#include "IR/Block.h"

#include <algorithm>

namespace Analysis {
	DominatorTree::DominatorTree(FlowGraph &flowGraph)
	{
		BlockSort sort(flowGraph);
		mBlocks = sort.sorted();

		mIDoms[mBlocks[0]] = mBlocks[0];

		bool changed;
		do {
			changed = false;
			for(unsigned int i=1; i<mBlocks.size(); i++) {
				FlowGraph::Block *block = mBlocks[i];
				FlowGraph::Block *newDom = 0;
				for(FlowGraph::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
					FlowGraph::Block *pred = *it;

					if(mIDoms[pred] == 0) {
						continue;
					}

					if(newDom) {
						FlowGraph::Block *a = pred;
						FlowGraph::Block *b = newDom;
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

	FlowGraph::Block *DominatorTree::idom(FlowGraph::Block *block)
	{
		return mIDoms[block];
	}

	const FlowGraph::BlockVector &DominatorTree::blocks() const
	{
		return mBlocks;
	}
}