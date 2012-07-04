#include "Dominance.h"

#include <algorithm>

DominatorTree::DominatorTree(std::vector<IR::Block*> &blocks)
{
	mBlocks = topologicalSort(blocks);
	std::map<IR::Block*, int> sortOrder;

	for(unsigned int i=0; i<mBlocks.size(); i++) {
		sortOrder[mBlocks[i]] = i;
	}

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
						while(sortOrder[a] > sortOrder[b])
							a = mIDoms[a];
						while(sortOrder[b] > sortOrder[a])
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

const std::vector<IR::Block*> &DominatorTree::blocks()
{
	return mBlocks;
}

void DominatorTree::topoSortRecurse(IR::Block *block, std::vector<IR::Block*> &blocks, std::set<IR::Block*> &seenBlocks)
{
	if(seenBlocks.find(block) != seenBlocks.end()) {
		return;
	}

	seenBlocks.insert(block);

	for(unsigned int i=0; i<block->succ.size(); i++) {
		topoSortRecurse(block->succ[i], blocks, seenBlocks);
	}

	blocks.push_back(block);
}

std::vector<IR::Block*> DominatorTree::topologicalSort(std::vector<IR::Block*> &blocks)
{
	std::set<IR::Block*> seenBlocks;
	std::vector<IR::Block*> result;

	for(unsigned int i=0; i<blocks.size(); i++) {
		topoSortRecurse(blocks[i], result, seenBlocks);
	}

	std::reverse(result.begin(), result.end());

	return result;
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

const std::set<IR::Block*> DominanceFrontiers::frontiers(IR::Block *block)
{
	return mFrontiers[block];
}