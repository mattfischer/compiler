#include "Analysis/Loops.h"

#include <queue>

namespace Analysis {
	Loops::Loops(FlowGraph &graph, DominatorTree &doms)
	{
		for(FlowGraph::BlockSet::iterator itBlock = graph.blocks().begin(); itBlock != graph.blocks().end(); itBlock++) {
			FlowGraph::Block *block = *itBlock;
			for(FlowGraph::BlockSet::iterator itSucc = block->succ.begin(); itSucc != block->succ.end(); itSucc++) {
				FlowGraph::Block *succ = *itSucc;
				if(isDominator(block, succ, doms)) {
					FlowGraph::Block *header = succ;
					FlowGraph::Block *bottom = block;
					Loop *loop = buildLoop(bottom, header);
					mLoops.push_back(loop);
					mLoopMap[header] = loop;
				}
			}
		}

		findParents(doms);
	}

	Loops::~Loops()
	{
		for(LoopList::iterator it = mLoops.begin(); it != mLoops.end(); it++) {
			Loop *loop = *it;
			delete loop;
		}
	}

	Loops::LoopList &Loops::loops()
	{
		return mLoops;
	}

	bool Loops::isDominator(FlowGraph::Block *block, FlowGraph::Block *test, DominatorTree &doms)
	{
		FlowGraph::Block *cursor = block;
		FlowGraph::Block *idom = doms.idom(cursor);

		while(idom != cursor) {
			cursor = idom;
			idom = doms.idom(cursor);
			if(cursor == test) {
				return true;
			}
		}

		return false;
	}

	Loops::Loop *Loops::buildLoop(FlowGraph::Block *bottom, FlowGraph::Block *header)
	{
		Loop *loop = new Loop;
		loop->header = header;
		loop->parent = 0;

		std::queue<FlowGraph::Block*> queue;
		queue.push(bottom);
		while(!queue.empty()) {
			FlowGraph::Block *block = queue.front();
			queue.pop();

			if(block == header || loop->blocks.find(block) != loop->blocks.end()) {
				continue;
			}

			loop->blocks.insert(block);

			for(FlowGraph::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
				FlowGraph::Block *pred = *it;
				queue.push(pred);
			}
		}

		loop->blocks.insert(header);

		return loop;
	}

	void Loops::findParents(DominatorTree &doms)
	{
		for(LoopList::iterator itLoop = mLoops.begin(); itLoop != mLoops.end(); itLoop++) {
			Loop *loop = *itLoop;

			FlowGraph::Block *block = loop->header;
			FlowGraph::Block *idom = doms.idom(block);

			while(idom != block) {
				block = idom;
				idom = doms.idom(block);

				BlockToLoopMap::iterator itMap = mLoopMap.find(block);
				if(itMap != mLoopMap.end()) {
					loop->parent = itMap->second;
					break;
				}
			}
		}
	}
}