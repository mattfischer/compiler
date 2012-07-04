#ifndef DOMINANCE_H
#define DOMINANCE_H

#include "IR.h"

#include <vector>
#include <map>
#include <set>

class DominatorTree {
public:
	DominatorTree(std::vector<IR::Block*> &blocks);

	const std::vector<IR::Block*> &blocks();
	IR::Block *idom(IR::Block *block);

private:
	void topoSortRecurse(IR::Block *block, std::vector<IR::Block*> &blocks, std::set<IR::Block*> &seenBlocks);
	std::vector<IR::Block*> topologicalSort(std::vector<IR::Block*> &blocks);

	std::vector<IR::Block*> mBlocks;
	std::map<IR::Block*, IR::Block*> mIDoms;
};

class DominanceFrontiers {
public:
	DominanceFrontiers(DominatorTree &tree);

	const std::set<IR::Block*> frontiers(IR::Block *block);

private:
	std::map<IR::Block*, std::set<IR::Block*> > mFrontiers;
};

#endif