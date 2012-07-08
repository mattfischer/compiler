#include "Dominance.h"

#include "Analysis/BlockSort.h"
#include "IR/Block.h"

#include <algorithm>

namespace Analysis {
	static DominanceFrontiers::BlockSet emptyBlockSet;

	DominatorTree::DominatorTree(BlockVector &blocks)
	{
		BlockSort sort(blocks);
		mBlocks = sort.sorted();

		mIDoms[mBlocks[0]] = mBlocks[0];

		bool changed;
		do {
			changed = false;
			for(unsigned int i=1; i<mBlocks.size(); i++) {
				IR::Block *block = mBlocks[i];
				IR::Block *newDom = 0;
				for(unsigned int j=0; j<block->pred.size(); j++) {
					IR::Block *pred = block->pred[j];

					if(mIDoms[pred] == 0) {
						continue;
					}

					if(newDom) {
						IR::Block *a = pred;
						IR::Block *b = newDom;
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

	IR::Block *DominatorTree::idom(IR::Block *block)
	{
		return mIDoms[block];
	}

	const DominatorTree::BlockVector &DominatorTree::blocks() const
	{
		return mBlocks;
	}

	DominanceFrontiers::DominanceFrontiers(DominatorTree &tree)
	{
		for(unsigned int i=0; i<tree.blocks().size(); i++) {
			IR::Block *block = tree.blocks()[i];

			if(block->pred.size() < 2)
				continue;

			for(unsigned int j=0; j<block->pred.size(); j++) {
				IR::Block *runner = block->pred[j];
				while(runner != tree.idom(block)) {
					mFrontiers[runner].insert(block);
					runner = tree.idom(runner);
				}
			}
		}
	}

	const DominanceFrontiers::BlockSet &DominanceFrontiers::frontiers(IR::Block *block) const
	{
		std::map<IR::Block*, BlockSet>::const_iterator it = mFrontiers.find(block);
		if(it != mFrontiers.end()) {
			return it->second;
		} else {
			return emptyBlockSet;
		}
	}
}