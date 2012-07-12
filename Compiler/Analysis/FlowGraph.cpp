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
			block->label = irBlock->label;
			block->end = irBlock->entries.back();
			mBlockSet.insert(block);
			mBlockMap[block->label] = block;
		}

		for(BlockSet::iterator it = mBlockSet.begin(); it != mBlockSet.end(); it++) {
			Block *block = *it;
			linkBlock(block);
		}

		mStart = mBlockMap[procedure->start()->label];
		mEnd = mBlockMap[procedure->end()->label];
	}

	FlowGraph::~FlowGraph()
	{
		for(BlockSet::iterator it = mBlockSet.begin(); it != mBlockSet.end(); it++) {
			Block *block = *it;
			delete block;
		}
	}

	void FlowGraph::linkBlock(Block *block)
	{
		mTailMap[block->end] = block;
		switch(block->end->type) {
			case IR::Entry::TypeJump:
				{
					IR::EntryJump *jump = (IR::EntryJump*)block->end;
					FlowGraph::Block *target = mBlockMap[jump->target];
					block->succ.insert(target);
					target->pred.insert(block);
					break;
				}
			case IR::Entry::TypeCJump:
				{
					IR::EntryCJump *cJump = (IR::EntryCJump*)block->end;
					FlowGraph::Block *trueTarget = mBlockMap[cJump->trueTarget];
					block->succ.insert(trueTarget);
					trueTarget->pred.insert(block);
					FlowGraph::Block *falseTarget = mBlockMap[cJump->falseTarget];
					block->succ.insert(falseTarget);
					falseTarget->pred.insert(block);
					break;
				}
		}
	}

	void FlowGraph::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		Block *block = mTailMap[oldEntry];
		for(BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
			Block *succ = *it;
			succ->pred.erase(block);
		}
		block->succ.clear();
		mTailMap.erase(oldEntry);
		block->end = newEntry;
		linkBlock(block);
	}
}