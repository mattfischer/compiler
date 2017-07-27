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
		typedef std::set<Block*> BlockSet;
		typedef std::vector<Block*> BlockVector;
		typedef std::list<Block*> BlockList;

		/*!
		 * \brief Block structure
		 */
		struct Block {
			BlockSet pred; //!< Predecessor blocks
			BlockSet succ; //!< Successor blocks
			IR::EntrySubList entries; //!< Entries in the block
		};

		FlowGraph(const IR::Procedure &procedure);
		~FlowGraph();

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);

		Block *start() const { return mStart; } //!< Start block
		Block *end() const { return mEnd; } //!< End block

		BlockSet &blocks() { return mBlockSet; } //!< Set of all blocks

	private:
		void linkBlock(Block *block, IR::Entry *back);

		typedef std::map<IR::Entry*, Block*> EntryToBlockMap;
		BlockSet mBlockSet; //!< Set of all blocks
		EntryToBlockMap mFrontMap; //!< Map of entries to the block that they are the front of
		EntryToBlockMap mBackMap; //!< Map of entries to the block that they are the back of
		Block *mStart; //!< Start block
		Block *mEnd; //!< End block
	};
}
#endif