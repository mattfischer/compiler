#ifndef ANALYSIS_DOMINANCE_FRONTIERS_H
#define ANALYSIS_DOMINANCE_FRONTIERS_H

#include "Analysis/FlowGraph.h"
#include "Analysis/DominatorTree.h"

#include <set>
#include <vector>

namespace Analysis {
	/*!
	 * \brief Determine the set of dominance frontiers in a procedure
	 *
	 * A dominance frontier of a block is the set of blocks which are not dominated by
	 * the block, but which have an immediate predecessor which is dominated by it.
	 */
	class DominanceFrontiers {
	public:
		DominanceFrontiers(DominatorTree &tree);

		const std::set<FlowGraph::Block*> &frontiers(FlowGraph::Block *block) const;

	private:
		std::map<FlowGraph::Block*, std::set<FlowGraph::Block*>> mFrontiers; //!< List of frontiers
	};
}
#endif