#ifndef ANALYSIS_FLOW_GRAPH_H
#define ANALYSIS_FLOW_GRAPH_H

#include <set>
#include <map>

namespace IR {
	class Procedure;
	class Block;
};

namespace Analysis {
	class FlowGraph {
	public:
		struct Block {
			typedef std::set<Block*> BlockSet;

			BlockSet pred;
			BlockSet succ;
			IR::Block *irBlock;
		};

		FlowGraph(IR::Procedure *procedure);
		~FlowGraph();

		const Block *start() const { return mStart; }
		const Block *end() const { return mEnd; }

		typedef std::set<Block*> BlockSet;
		const BlockSet &blocks() const { return mBlockSet; }

	private:
		typedef std::map<IR::Block*, Block*> IRBlockToBlockMap;
		BlockSet mBlockSet;
		IRBlockToBlockMap mBlockMap;
		Block *mStart;
		Block *mEnd;
	};
}
#endif