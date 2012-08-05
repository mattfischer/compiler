#ifndef ANALYSIS_LOOPS_H
#define ANALYSIS_LOOPS_H

#include "Analysis/FlowGraph.h"
#include "Analysis/DominatorTree.h"

#include <list>
#include <map>
#include <set>

namespace Analysis {
	class Loops {
	public:
		struct Loop;
		typedef std::list<Loop*> LoopList;
		typedef std::set<Loop*> LoopSet;

		struct Loop {
			FlowGraph::Block *header;
			FlowGraph::Block *preheader;
			FlowGraph::BlockSet blocks;
			Loop *parent;
			LoopSet children;
		};

		Loops(IR::Procedure *procedure);
		~Loops();

		LoopList &loops();
		Loop *rootLoop() { return &mRootLoop; }

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
		Loop mRootLoop;
	};
}
#endif