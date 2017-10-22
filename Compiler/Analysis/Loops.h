#ifndef ANALYSIS_LOOPS_H
#define ANALYSIS_LOOPS_H

#include "Analysis/FlowGraph.h"
#include "Analysis/DominatorTree.h"

#include <list>
#include <map>
#include <set>
#include <iostream>

namespace Analysis {
	/*!
	 * \brief Find loops in a procedure
	 *
	 * A loop is any collection of blocks in the control flow graph where the bottom-most block
	 * connects back to the top block.  Loops may be nested inside of other loops, therefore the
	 * set of loops in a procedure form a tree.
	 */
	class Loops {
	public:
		struct Loop;

		/*!
		 * \brief Represents a single loop
		 */
		struct Loop {
			const FlowGraph::Block *header; //!< First block in the loop
			const FlowGraph::Block *preheader; //!< Block preceeding the first block in the loop
			std::set<const FlowGraph::Block*> blocks; //!< Set of blocks in the loop
			Loop *parent; //!< Loop which contains this loop
			std::set<Loop*> children; //!< Loops inside of this loop
		};

		Loops(const IR::Procedure &procedure, const FlowGraph &flowGraph);

		std::list<std::unique_ptr<Loop>> &loops();
		Loop *rootLoop() { return &mRootLoop; } //!< Loop representing the entire procedure

		void print(std::ostream &o);

	private:
		bool isDominator(const FlowGraph::Block *block, const FlowGraph::Block *test, const DominatorTree &doms);
		std::unique_ptr<Loop> buildLoop(const FlowGraph::Block *bottom, const FlowGraph::Block *header);
		void findParents(const DominatorTree &doms);
		const FlowGraph::Block *findPreheader(Loop &loop);

		std::list<std::unique_ptr<Loop>> mLoops; //!< List of all loops
		std::map<const FlowGraph::Block*, Loop*> mLoopMap; //!< Map from blocks to the loop they are the header of
		Loop mRootLoop; //!< Root loop of the procedure
	};
}
#endif