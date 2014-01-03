#include "Analysis/FlowGraph.h"

#include "IR/Procedure.h"

namespace Analysis {
	FlowGraph::FlowGraph(IR::Procedure *procedure)
	{
		Block *block = 0;
		IR::EntrySubList::iterator begin;
		IR::EntrySubList::iterator end;

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::TypeLabel:
					if(block) {
						end = itEntry;
						block->entries = IR::EntrySubList(begin, end);
						mBlockSet.insert(block);
						mFrontMap[block->entries.front()] = block;
					}
					block = new Block;
					begin = itEntry;
					break;

				case IR::Entry::TypeJump:
				case IR::Entry::TypeCJump:
					end = itEntry;
					end++;
					block->entries = IR::EntrySubList(begin, end);
					mBlockSet.insert(block);
					mFrontMap[block->entries.front()] = block;
					block = 0;
					break;
			}
		}

		if(block) {
			end = procedure->entries().end();
			block->entries = IR::EntrySubList(begin, end);
			mBlockSet.insert(block);
			mFrontMap[block->entries.front()] = block;
		}

		mStart = mFrontMap[procedure->start()];
		mEnd = mFrontMap[procedure->end()];

		for(BlockSet::iterator it = mBlockSet.begin(); it != mBlockSet.end(); it++) {
			Block *block = *it;
			linkBlock(block, block->entries.back());
		}
	}

	FlowGraph::~FlowGraph()
	{
		for(BlockSet::iterator it = mBlockSet.begin(); it != mBlockSet.end(); it++) {
			Block *block = *it;
			delete block;
		}
	}

	void FlowGraph::linkBlock(Block *block, IR::Entry *back)
	{
		mBackMap[back] = block;
		switch(back->type) {
			case IR::Entry::TypeJump:
				{
					IR::EntryJump *jump = (IR::EntryJump*)back;
					FlowGraph::Block *target = mFrontMap[jump->target];
					block->succ.insert(target);
					target->pred.insert(block);
					break;
				}
			case IR::Entry::TypeCJump:
				{
					IR::EntryCJump *cJump = (IR::EntryCJump*)back;
					FlowGraph::Block *trueTarget = mFrontMap[cJump->trueTarget];
					block->succ.insert(trueTarget);
					trueTarget->pred.insert(block);
					FlowGraph::Block *falseTarget = mFrontMap[cJump->falseTarget];
					block->succ.insert(falseTarget);
					falseTarget->pred.insert(block);
					break;
				}
			case IR::Entry::TypeReturn:
				{
					block->succ.insert(mEnd);
					mEnd->pred.insert(block);
					break;
				}
			default:
				{
					IR::Entry *nextLabel = *block->entries.end();
					Block *succ = mFrontMap[nextLabel];
					if(succ) {
						block->succ.insert(succ);
						succ->pred.insert(block);
					}
					break;
				}
		}
	}

	void FlowGraph::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		Block *block = mBackMap[oldEntry];
		if(block) {
			for(BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
				Block *succ = *it;
				succ->pred.erase(block);
			}
			block->succ.clear();
			mBackMap.erase(oldEntry);
			linkBlock(block, newEntry);
		}
	}
}