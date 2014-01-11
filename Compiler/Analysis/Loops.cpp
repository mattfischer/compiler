#include "Analysis/Loops.h"

#include <queue>

namespace Analysis {
	Loops::Loops(IR::Procedure *procedure)
		: mFlowGraph(procedure)
	{
		DominatorTree dominatorTree(procedure, mFlowGraph);
		mRootLoop.parent = &mRootLoop;
		mRootLoop.header = mFlowGraph.start();
		for(FlowGraph::BlockSet::iterator itBlock = mFlowGraph.blocks().begin(); itBlock != mFlowGraph.blocks().end(); itBlock++) {
			FlowGraph::Block *block = *itBlock;
			mRootLoop.blocks.insert(block);
		}
		mLoopMap[mRootLoop.header] = &mRootLoop;

		for(FlowGraph::BlockSet::iterator itBlock = mFlowGraph.blocks().begin(); itBlock != mFlowGraph.blocks().end(); itBlock++) {
			FlowGraph::Block *block = *itBlock;
			for(FlowGraph::BlockSet::iterator itSucc = block->succ.begin(); itSucc != block->succ.end(); itSucc++) {
				FlowGraph::Block *succ = *itSucc;
				if(dominatorTree.dominates(block, succ)) {
					FlowGraph::Block *header = succ;
					FlowGraph::Block *bottom = block;
					Loop *loop = buildLoop(bottom, header);
					mLoops.push_back(loop);
					mLoopMap[header] = loop;
				}
			}
		}

		findParents(dominatorTree);
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
		loop->preheader = findPreheader(loop);

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
					loop->parent->children.insert(loop);
					break;
				}
			}
		}
	}

	FlowGraph::Block *Loops::findPreheader(Loop *loop)
	{
		FlowGraph::Block *preheader = 0;
		for(FlowGraph::BlockSet::iterator itBlock = loop->header->pred.begin(); itBlock != loop->header->pred.end(); itBlock++) {
			FlowGraph::Block *block = *itBlock;
			if(loop->blocks.find(block) != loop->blocks.end()) {
				continue;
			}

			if(preheader) {
				preheader = 0;
				break;
			} else {
				preheader = block;
			}
		}

		if(!preheader || preheader->succ.size() > 1) {
			preheader = 0;
		}

		return preheader;
	}

	void Loops::print()
	{
		std::map<Loop *, int> loopMap;
		int num = 1;
		for(LoopList::iterator itLoop = mLoops.begin(); itLoop != mLoops.end(); itLoop++) {
			Loop *loop = *itLoop;
			loopMap[loop] = num++;
		}

		for(LoopList::iterator itLoop = mLoops.begin(); itLoop != mLoops.end(); itLoop++) {
			Loop *loop = *itLoop;
			std::cout << loopMap[loop] << ": ";
			if(loop->parent != &mRootLoop) {
				std::cout << "parent: " << loopMap[loop->parent] << " | ";
			}
			std::cout << "header: " << ((IR::EntryLabel*)loop->header->entries.front())->name << " | ";
			if(loop->preheader) {
				std::cout << "preheader: " << ((IR::EntryLabel*)loop->preheader->entries.front())->name << " | ";
			}
			std::cout << "blocks: ";
			for(FlowGraph::BlockSet::iterator itBlock = loop->blocks.begin(); itBlock != loop->blocks.end(); itBlock++) {
				FlowGraph::Block *block = *itBlock;
				std::cout << ((IR::EntryLabel*)block->entries.front())->name << " ";
			}
			std::cout << std::endl;
		}
	}
}