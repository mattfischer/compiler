#include "Analysis/FlowGraph.h"

#include "IR/Procedure.h"

namespace Analysis {
	FlowGraph::FlowGraph(IR::Procedure *procedure)
	{
		Block *block = 0;
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeLabel:
					if(block) {
						block->end = entry;
						mBlockSet.insert(block);
						mBlockMap[block->label] = block;
					}
					block = new Block;
					block->label = (IR::EntryLabel*)entry;
					break;

				case IR::Entry::TypeJump:
				case IR::Entry::TypeCJump:
					block->end = entry;
					mBlockSet.insert(block);
					mBlockMap[block->label] = block;
					block = 0;
					break;
			}
		}

		for(BlockSet::iterator it = mBlockSet.begin(); it != mBlockSet.end(); it++) {
			Block *block = *it;
			linkBlock(block);
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

	void FlowGraph::linkBlock(Block *block)
	{
		if(!block->end) {
			return;
		}

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