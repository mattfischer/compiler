#ifndef ANALYSIS_FLOW_GRAPH_H
#define ANALYSIS_FLOW_GRAPH_H

#include <set>
#include <vector>
#include <map>

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
			IR::Block *irBlock;
		};

		FlowGraph(IR::Procedure *procedure);
		~FlowGraph();

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);

		Block *start() const { return mStart; }
		Block *end() const { return mEnd; }

		BlockSet &blocks() { return mBlockSet; }

	private:
		void addTail(Block *block, IR::Entry *entry);

		typedef std::map<IR::Block*, Block*> IRBlockToBlockMap;
		BlockSet mBlockSet;
		IRBlockToBlockMap mBlockMap;
		Block *mStart;
		Block *mEnd;
		typedef std::map<IR::Entry*, Block*> EntryToBlockMap;
		EntryToBlockMap mTailMap;
	};
}
#endif