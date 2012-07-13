#ifndef ANALYSIS_FLOW_GRAPH_H
#define ANALYSIS_FLOW_GRAPH_H

#include <set>
#include <vector>
#include <map>

#include "IR/Entry.h"
#include "IR/EntryList.h"
#include "IR/EntrySubList.h"

namespace IR {
	class Entry;
	class Procedure;
};

namespace Analysis {
	class FlowGraph {
	public:
		struct Block;
		typedef std::set<Block*> BlockSet;
		typedef std::vector<Block*> BlockVector;

		struct Block {
			BlockSet pred;
			BlockSet succ;
			IR::EntrySubList entries;
		};

		FlowGraph(IR::Procedure *procedure);
		~FlowGraph();

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);

		Block *start() const { return mStart; }
		Block *end() const { return mEnd; }

		BlockSet &blocks() { return mBlockSet; }

	private:
		void linkBlock(Block *block, IR::Entry *back);

		typedef std::map<IR::Entry*, Block*> EntryToBlockMap;
		BlockSet mBlockSet;
		EntryToBlockMap mFrontMap;
		EntryToBlockMap mBackMap;
		Block *mStart;
		Block *mEnd;
	};
}
#endif