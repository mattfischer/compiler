#ifndef ANALYSIS_LOOPS_H
#define ANALYSIS_LOOPS_H

#include "Analysis/FlowGraph.h"
#include "Analysis/DominatorTree.h"

#include <list>
#include <map>

namespace Analysis {
	class Loops {
	public:
		struct Loop {
			FlowGraph::Block *header;
			FlowGraph::Block *preheader;
			FlowGraph::BlockSet blocks;
			Loop *parent;
		};

		Loops(IR::Procedure *procedure);
		~Loops();

		typedef std::list<Loop*> LoopList;
		LoopList &loops();

		void print();

	private:
		bool isDominator(FlowGraph::Block *block, FlowGraph::Block *test, DominatorTree &doms);
		Loop *buildLoop(FlowGraph::Block *bottom, FlowGraph::Block *header);
		void findParents(DominatorTree &doms);
		FlowGraph::Block *findPreheader(Loop *loop);

		LoopList mLoops;
		typedef std::map<FlowGraph::Block*, Loop*> BlockToLoopMap;
		BlockToLoopMap mLoopMap;
		FlowGraph mFlowGraph;
	};
}
#endif