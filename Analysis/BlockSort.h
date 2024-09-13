#ifndef ANALYSIS_BLOCK_SORT_H
#define ANALYSIS_BLOCK_SORT_H

#include "Analysis/FlowGraph.h"

#include <vector>
#include <set>
#include <map>

namespace Analysis {
	/*!
	 * \brief Topologically sort a control flow graph
	 */
	class BlockSort {
	public:
		BlockSort(const FlowGraph &flowGraph);

		const std::vector<const FlowGraph::Block*> &sorted() const;
		int position(const FlowGraph::Block *block) const;

	private:
		void sortRecurse(const FlowGraph::Block *block, std::vector<const FlowGraph::Block*> &blocks, std::set<const FlowGraph::Block*> &seenBlocks);

		std::vector<const FlowGraph::Block*> mSorted; //!< Sorted block list
		std::map<const FlowGraph::Block*, int> mOrder; //!< Mapping from block to position in list
	};
}
#endif