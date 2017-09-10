#include "Analysis/FlowGraph.h"

#include "IR/Procedure.h"

namespace Analysis {
	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 */
	FlowGraph::FlowGraph(const IR::Procedure &procedure)
	{
		std::unique_ptr<Block> block;
		IR::EntrySubList::iterator begin;
		IR::EntrySubList::iterator end;

		// First, break up the procedure into blocks
		for(IR::EntryList::iterator itEntry = const_cast<IR::Procedure&>(procedure).entries().begin(); itEntry != const_cast<IR::Procedure&>(procedure).entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			switch(entry->type) {
				case IR::Entry::Type::Label:
					// A label forces the start of a new block
					if(block) {
						// If a previous block was under construction, it is now complete.
						// Construct its entry list, now that we know its endpoint, and add
						// it to the list.
						end = itEntry;
						block->entries = IR::EntrySubList(begin, end);
						mFrontMap[block->entries.front()] = block.get();
						mBlocks.push_back(std::move(block));
					}
					// Start a new block, beginning with the label just encountered
					block = std::make_unique<Block>();
					begin = itEntry;
					break;

				case IR::Entry::Type::Jump:
				case IR::Entry::Type::CJump:
					// A jump instruction forces the end of the current block.  Construct
					// its entry list now that we know its endpoint, and add it to the list.
					end = itEntry;
					end++;
					block->entries = IR::EntrySubList(begin, end);
					mFrontMap[block->entries.front()] = block.get();
					mBlocks.push_back(std::move(block));
					block = 0;
					break;
			}
		}

		// Add the final block, if present
		if(block) {
			end = const_cast<IR::Procedure&>(procedure).entries().end();
			block->entries = IR::EntrySubList(begin, end);
			mFrontMap[block->entries.front()] = block.get();
			mBlocks.push_back(std::move(block));
		}

		// Save off the start and end pointers
		mStart = mFrontMap[procedure.start()];
		mEnd = mFrontMap[procedure.end()];

		// Now loop through the blocks and construct the links between them
		for(std::unique_ptr<Block> &block : mBlocks) {
			linkBlock(block.get(), block->entries.back());
		}
	}

	/*!
	 * \brief Link a block to its predecessors and successors
	 * \param block Block to link
	 * \param back Back entry of the block
	 */
	void FlowGraph::linkBlock(Block *block, IR::Entry *back)
	{
		// Examine the back entry in the block to determine which blocks it links to
		mBackMap[back] = block;
		switch(back->type) {
			case IR::Entry::Type::Jump:
				{
					// Unconditional jump.  Link the block to the jump target
					IR::EntryJump *jump = (IR::EntryJump*)back;
					FlowGraph::Block *target = mFrontMap[jump->target];
					block->succ.insert(target);
					target->pred.insert(block);
					break;
				}
			case IR::Entry::Type::CJump:
				{
					// Conditional jump.  Link th block to the true and false jump targets
					IR::EntryCJump *cJump = (IR::EntryCJump*)back;
					FlowGraph::Block *trueTarget = mFrontMap[cJump->trueTarget];
					block->succ.insert(trueTarget);
					trueTarget->pred.insert(block);
					FlowGraph::Block *falseTarget = mFrontMap[cJump->falseTarget];
					block->succ.insert(falseTarget);
					falseTarget->pred.insert(block);
					break;
				}
			default:
				{
					// The block ended with a non-jump instruction.  Link the block to the
					// block started by the next entry
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

	/*!
	 * \brief Replace an entry with a new entry, updating graph edges as necessary
	 * \param oldEntry Entry to replace
	 * \param newEntry Entry to replace with
	 */
	void FlowGraph::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		// Check if entry was the back of a block
		Block *block = mBackMap[oldEntry];
		if(block) {
			// Break links with all successor blocks
			for(Block *succ : block->succ) {
				succ->pred.erase(block);
			}
			block->succ.clear();
			mBackMap.erase(oldEntry);

			// Re-link the block with respect to the new entry
			linkBlock(block, newEntry);
		}
	}
}