#ifndef ANALYSIS_BLOCK_SORT_H
#define ANALYSIS_BLOCK_SORT_H

#include <vector>
#include <set>
#include <map>

namespace IR {
	class Block;
}

namespace Analysis {
	class BlockSort {
	public:
		typedef std::vector<IR::Block*> BlockVector;
		BlockSort(const BlockVector &blocks);

		const BlockVector &sorted() const;
		int position(IR::Block *block) const;

	private:
		typedef std::set<IR::Block*> BlockSet;
		void sortRecurse(IR::Block *block, BlockVector &blocks, BlockSet &seenBlocks);

		BlockVector mSorted;
		typedef std::map<IR::Block*, int> OrderMap;
		OrderMap mOrder;
	};
}
#endif