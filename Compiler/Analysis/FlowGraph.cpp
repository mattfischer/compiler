#include "Analysis/FlowGraph.h"

#include "IR/Procedure.h"
#include "IR/Block.h"

namespace Analysis {
	FlowGraph::FlowGraph(IR::Procedure *procedure)
	{
		for(unsigned int i=0; i<procedure->blocks().size(); i++) {
			IR::Block *irBlock = procedure->blocks()[i];

			Block *block = new Block;
			block->irBlock = irBlock;
			mBlockSet.insert(block);
			mBlockMap[irBlock] = block;
		}

		for(BlockSet::iterator it = mBlockSet.begin(); it != mBlockSet.end(); it++) {
			Block *block = *it;
			IR::Block *irBlock = block->irBlock;

			for(IR::Block::BlockSet::iterator it2 = irBlock->pred.begin(); it2 != irBlock->pred.end(); it2++) {
				IR::Block *pred = *it2;
				block->pred.insert(mBlockMap[pred]);
			}

			for(IR::Block::BlockSet::iterator it2 = irBlock->succ.begin(); it2 != irBlock->succ.end(); it2++) {
				IR::Block *succ = *it2;
				block->succ.insert(mBlockMap[succ]);
			}
		}

		mStart = mBlockMap[procedure->start()];
		mEnd = mBlockMap[procedure->end()];
	}

	FlowGraph::~FlowGraph()
	{
		for(BlockSet::iterator it = mBlockSet.begin(); it != mBlockSet.end(); it++) {
			Block *block = *it;
			delete block;
		}
	}
}