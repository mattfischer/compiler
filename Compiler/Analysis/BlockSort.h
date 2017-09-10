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
		BlockSort(FlowGraph &flowGraph);

		const std::vector<FlowGraph::Block*> &sorted() const;
		int position(FlowGraph::Block *block) const;

	private:
		void sortRecurse(FlowGraph::Block *block, std::vector<FlowGraph::Block*> &blocks, std::set<FlowGraph::Block*> &seenBlocks);

		std::vector<FlowGraph::Block*> mSorted; //!< Sorted block list
		std::map<FlowGraph::Block*, int> mOrder; //!< Mapping from block to position in list
	};
}
#endif