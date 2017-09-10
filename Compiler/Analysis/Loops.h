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
			FlowGraph::Block *header; //!< First block in the loop
			FlowGraph::Block *preheader; //!< Block preceeding the first block in the loop
			std::set<FlowGraph::Block*> blocks; //!< Set of blocks in the loop
			Loop *parent; //!< Loop which contains this loop
			std::set<Loop*> children; //!< Loops inside of this loop
		};

		Loops(const IR::Procedure &procedure, FlowGraph &flowGraph);

		std::list<std::unique_ptr<Loop>> &loops();
		Loop *rootLoop() { return &mRootLoop; } //!< Loop representing the entire procedure

		void print(std::ostream &o);

	private:
		bool isDominator(FlowGraph::Block *block, FlowGraph::Block *test, DominatorTree &doms);
		std::unique_ptr<Loop> buildLoop(FlowGraph::Block *bottom, FlowGraph::Block *header);
		void findParents(DominatorTree &doms);
		FlowGraph::Block *findPreheader(Loop &loop);

		std::list<std::unique_ptr<Loop>> mLoops; //!< List of all loops
		std::map<FlowGraph::Block*, Loop*> mLoopMap; //!< Map from blocks to the loop they are the header of
		Loop mRootLoop; //!< Root loop of the procedure
	};
}
#endif