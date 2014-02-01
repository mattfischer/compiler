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
		BlockSort(FlowGraph *flowGraph);

		const FlowGraph::BlockVector &sorted() const;
		int position(FlowGraph::Block *block) const;

	private:
		void sortRecurse(FlowGraph::Block *block, FlowGraph::BlockVector &blocks, FlowGraph::BlockSet &seenBlocks);

		FlowGraph::BlockVector mSorted; //!< Sorted block list
		typedef std::map<FlowGraph::Block*, int> OrderMap;
		OrderMap mOrder; //!< Mapping from block to position in list
	};
}
#endif