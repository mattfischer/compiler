#ifndef ANALYSIS_DOMINANCE_H
#define ANALYSIS_DOMINANCE_H

#include <vector>
#include <map>
#include <set>

namespace IR {
	struct Block;
}

namespace Analysis {
	class DominatorTree {
	public:
		typedef std::vector<IR::Block*> BlockVector;

		DominatorTree(BlockVector &blocks);

		const BlockVector &blocks() const;
		IR::Block *idom(IR::Block *block);

	private:
		typedef std::set<IR::Block*> BlockSet;
		void topoSortRecurse(IR::Block *block, BlockVector &blocks, BlockSet &seenBlocks);
		BlockVector topologicalSort(BlockVector &blocks);

		BlockVector mBlocks;
		std::map<IR::Block*, IR::Block*> mIDoms;
	};

	class DominanceFrontiers {
	public:
		DominanceFrontiers(DominatorTree &tree);

		typedef std::set<IR::Block*> BlockSet;
		typedef std::vector<IR::Block*> BlockVector;
		const BlockSet &frontiers(IR::Block *block) const;

	private:
		std::map<IR::Block*, BlockSet> mFrontiers;
	};
}

#endif