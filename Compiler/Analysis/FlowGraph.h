#ifndef ANALYSIS_FLOW_GRAPH_H
#define ANALYSIS_FLOW_GRAPH_H

#include <set>
#include <vector>
#include <map>

#include "IR/Entry.h"

namespace IR {
	class Entry;
	class Procedure;
	class Block;
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
			IR::EntryLabel *label;
			IR::Entry *end;
			IR::Block *irBlock;
		};

		FlowGraph(IR::Procedure *procedure);
		~FlowGraph();

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);

		Block *start() const { return mStart; }
		Block *end() const { return mEnd; }

		BlockSet &blocks() { return mBlockSet; }

	private:
		void linkBlock(Block *block);

		typedef std::map<IR::EntryLabel*, Block*> LabelToBlockMap;
		BlockSet mBlockSet;
		LabelToBlockMap mBlockMap;
		Block *mStart;
		Block *mEnd;
		typedef std::map<IR::Entry*, Block*> EntryToBlockMap;
		EntryToBlockMap mTailMap;
	};
}
#endif