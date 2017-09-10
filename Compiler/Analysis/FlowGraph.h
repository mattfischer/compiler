#ifndef ANALYSIS_FLOW_GRAPH_H
#define ANALYSIS_FLOW_GRAPH_H

#include "IR/Entry.h"
#include "IR/EntryList.h"
#include "IR/EntrySubList.h"
#include "IR/Procedure.h"

#include <set>
#include <vector>
#include <map>
#include <list>

namespace Analysis {
	/*!
	 * \brief Control flow graph
	 *
	 * Calculates the control flow graph for a procedure.  The control flow graph is a
	 * set of basic blocks, along with the set of predecessor and successor blocks that
	 * each block connects to.  Additionally, the graph has a special start and end block
	 * that represent the beginning and end of the procedure.
	 */
	class FlowGraph {
	public:
		struct Block;

		/*!
		 * \brief Block structure
		 */
		struct Block {
			std::set<Block*> pred; //!< Predecessor blocks
			std::set<Block*> succ; //!< Successor blocks
			IR::EntrySubList entries; //!< Entries in the block
		};

		FlowGraph(const IR::Procedure &procedure);

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);

		Block *start() const { return mStart; } //!< Start block
		Block *end() const { return mEnd; } //!< End block

		std::vector<std::unique_ptr<Block>> &blocks() { return mBlocks; } //!< Set of all blocks

	private:
		void linkBlock(Block *block, IR::Entry *back);

		std::vector<std::unique_ptr<Block>> mBlocks; //!< Set of all blocks
		std::map<IR::Entry*, Block*> mFrontMap; //!< Map of entries to the block that they are the front of
		std::map<IR::Entry*, Block*> mBackMap; //!< Map of entries to the block that they are the back of
		Block *mStart; //!< Start block
		Block *mEnd; //!< End block
	};
}
#endif