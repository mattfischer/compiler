#include "Analysis/Loops.h"

#include <queue>

namespace Analysis {
	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 */
	Loops::Loops(IR::Procedure *procedure, FlowGraph *flowGraph)
	{
		// Construct a dominator tree for the procedure
		DominatorTree dominatorTree(procedure, flowGraph);

		// Construct the root loop
		mRootLoop.parent = &mRootLoop;
		mRootLoop.header = flowGraph->start();
		for(FlowGraph::BlockSet::iterator itBlock = flowGraph->blocks().begin(); itBlock != flowGraph->blocks().end(); itBlock++) {
			FlowGraph::Block *block = *itBlock;
			mRootLoop.blocks.insert(block);
		}
		mLoopMap[mRootLoop.header] = &mRootLoop;

		// Iterate through the graph, looking for loops
		for(FlowGraph::BlockSet::iterator itBlock = flowGraph->blocks().begin(); itBlock != flowGraph->blocks().end(); itBlock++) {
			FlowGraph::Block *block = *itBlock;
			for(FlowGraph::BlockSet::iterator itSucc = block->succ.begin(); itSucc != block->succ.end(); itSucc++) {
				FlowGraph::Block *succ = *itSucc;
				// If a block's successor dominates the block, then the successor is the head of a loop
				if(dominatorTree.dominates(block, succ)) {
					// Build a new loop out of the blocks found
					FlowGraph::Block *header = succ;
					FlowGraph::Block *bottom = block;
					Loop *loop = buildLoop(bottom, header);
					mLoops.push_back(loop);
					mLoopMap[header] = loop;
				}
			}
		}

		// Assign the loops to their proper locations in the loop tree
		findParents(dominatorTree);
	}

	/*!
	 * \brief Destructor
	 */
	Loops::~Loops()
	{
		for(LoopList::iterator it = mLoops.begin(); it != mLoops.end(); it++) {
			Loop *loop = *it;
			delete loop;
		}
	}

	/*!
	 * \brief Return list of all loops
	 * \return Loop list
	 */
	Loops::LoopList &Loops::loops()
	{
		return mLoops;
	}

	/*!
	 * \brief Construct a loop
	 * \param bottom Bottom-most block in the loop
	 * \param header Top block in the loop
	 * \return Newly-constructed loop
	 */
	Loops::Loop *Loops::buildLoop(FlowGraph::Block *bottom, FlowGraph::Block *header)
	{
		// Construct the new loop
		Loop *loop = new Loop;
		loop->header = header;
		loop->parent = 0;

		// Populate the loop's block list, starting from the bottom
		std::queue<FlowGraph::Block*> queue;
		queue.push(bottom);
		while(!queue.empty()) {
			FlowGraph::Block *block = queue.front();
			queue.pop();

			// If the block is either the loop header, or is already in the loop, then it
			// does not need to be followed any further
			if(block == header || loop->blocks.find(block) != loop->blocks.end()) {
				continue;
			}

			loop->blocks.insert(block);

			// Continue following the block's predecessors
			for(FlowGraph::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
				FlowGraph::Block *pred = *it;
				queue.push(pred);
			}
		}

		// Add the blocks found above to the loop
		loop->blocks.insert(header);

		// Find the loop's preheader
		loop->preheader = findPreheader(loop);

		return loop;
	}

	/*!
	 * \brief Construct the tree structure of the loops
	 * \param doms Dominator tree for the control flow graph
	 */
	void Loops::findParents(DominatorTree &doms)
	{
		// Iterate through the list of loops
		for(LoopList::iterator itLoop = mLoops.begin(); itLoop != mLoops.end(); itLoop++) {
			Loop *loop = *itLoop;

			FlowGraph::Block *block = loop->header;
			FlowGraph::Block *idom = doms.idom(block);

			// Seach upward through the dominator tree, looking for a block which is
			// the header of a loop
			while(idom != block) {
				block = idom;
				idom = doms.idom(block);

				BlockToLoopMap::iterator itMap = mLoopMap.find(block);
				if(itMap != mLoopMap.end()) {
					// If this block is the header of a loop, then that loop is the parent of this one
					loop->parent = itMap->second;
					loop->parent->children.insert(loop);
					break;
				}
			}
		}
	}

	/*!
	 * \brief Find the preheader of a loop, the block directly before the loop begins
	 * \param loop Loop to examine
	 * \return Loop preheader, or 0 if there is no unique preheader block
	 */
	FlowGraph::Block *Loops::findPreheader(Loop *loop)
	{
		FlowGraph::Block *preheader = 0;

		// Search through the predecessors of the header block
		for(FlowGraph::BlockSet::iterator itBlock = loop->header->pred.begin(); itBlock != loop->header->pred.end(); itBlock++) {
			FlowGraph::Block *block = *itBlock;
			if(loop->blocks.find(block) != loop->blocks.end()) {
				// This block is still in the loop, so it can't be the preheader
				continue;
			}

			// The block is a viable preheader to the loop

			if(preheader) {
				// A preheader was already found, so there is no unique preheader block for this loop
				preheader = 0;
				break;
			} else {
				// No preheader was already found, so record this block as the preheader
				preheader = block;
			}
		}

		if(!preheader || preheader->succ.size() > 1) {
			// If no or multiple preheaders were found, return 0
			preheader = 0;
		}

		return preheader;
	}

	/*!
	 * \brief Print the list of loops
	 */
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