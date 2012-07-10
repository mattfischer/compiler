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
			IR::Entry *tail = irBlock->tail()->prev;

			addTail(block, tail);
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

	void FlowGraph::addTail(Block *block, IR::Entry *entry)
	{
		mTailMap[entry] = block;
		if(entry->type == IR::Entry::TypeJump) {
			IR::EntryJump *jump = (IR::EntryJump*)entry;
			FlowGraph::Block *target = mBlockMap[jump->target];
			block->succ.insert(target);
			target->pred.insert(block);
		} else if(entry->type == IR::Entry::TypeCJump) {
			IR::EntryCJump *cJump = (IR::EntryCJump*)entry;
			FlowGraph::Block *trueTarget = mBlockMap[cJump->trueTarget];
			block->succ.insert(trueTarget);
			trueTarget->pred.insert(block);
			FlowGraph::Block *falseTarget = mBlockMap[cJump->falseTarget];
			block->succ.insert(falseTarget);
			falseTarget->pred.insert(block);
		}
	}

	void FlowGraph::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		Block *block = mTailMap[oldEntry];
		for(Block::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
			Block *succ = *it;
			succ->pred.erase(block);
		}
		block->succ.clear();
		mTailMap.erase(oldEntry);
		addTail(block, newEntry);
	}
}