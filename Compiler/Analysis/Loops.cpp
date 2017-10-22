#include "Analysis/Loops.h"

#include <queue>

namespace Analysis {
	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 */
	Loops::Loops(const IR::Procedure &procedure, const FlowGraph &flowGraph)
	{
		// Construct a dominator tree for the procedure
		DominatorTree dominatorTree(procedure, flowGraph);

		// Construct the root loop
		mRootLoop.parent = &mRootLoop;
		mRootLoop.header = flowGraph.start();
		for(const std::unique_ptr<FlowGraph::Block> &block : flowGraph.blocks()) {
			mRootLoop.blocks.insert(block.get());
		}
		mLoopMap[mRootLoop.header] = &mRootLoop;

		// Iterate through the graph, looking for loops
		for(const std::unique_ptr<FlowGraph::Block> &block : flowGraph.blocks()) {
			for(const FlowGraph::Block *succ : block->succ) {
				// If a block's successor dominates the block, then the successor is the head of a loop
				if(dominatorTree.dominates(block.get(), succ)) {
					// Build a new loop out of the blocks found
					const FlowGraph::Block *header = succ;
					const FlowGraph::Block *bottom = block.get();
					std::unique_ptr<Loop> loop = buildLoop(bottom, header);
					mLoopMap[header] = loop.get();
					mLoops.push_back(std::move(loop));
				}
			}
		}

		// Assign the loops to their proper locations in the loop tree
		findParents(dominatorTree);
	}

	/*!
	 * \brief Return list of all loops
	 * \return Loop list
	 */
	std::list<std::unique_ptr<Loops::Loop>> &Loops::loops()
	{
		return mLoops;
	}

	/*!
	 * \brief Construct a loop
	 * \param bottom Bottom-most block in the loop
	 * \param header Top block in the loop
	 * \return Newly-constructed loop
	 */
	std::unique_ptr<Loops::Loop> Loops::buildLoop(const FlowGraph::Block *bottom, const FlowGraph::Block *header)
	{
		// Construct the new loop
		std::unique_ptr<Loop> loop = std::make_unique<Loop>();
		loop->header = header;
		loop->parent = 0;

		// Populate the loop's block list, starting from the bottom
		std::queue<const FlowGraph::Block*> queue;
		queue.push(bottom);
		while(!queue.empty()) {
			const FlowGraph::Block *block = queue.front();
			queue.pop();

			// If the block is either the loop header, or is already in the loop, then it
			// does not need to be followed any further
			if(block == header || loop->blocks.find(block) != loop->blocks.end()) {
				continue;
			}

			loop->blocks.insert(block);

			// Continue following the block's predecessors
			for(const FlowGraph::Block *pred : block->pred) {
				queue.push(pred);
			}
		}

		// Add the blocks found above to the loop
		loop->blocks.insert(header);

		// Find the loop's preheader
		loop->preheader = findPreheader(*loop);

		return loop;
	}

	/*!
	 * \brief Construct the tree structure of the loops
	 * \param doms Dominator tree for the control flow graph
	 */
	void Loops::findParents(DominatorTree &doms)
	{
		// Iterate through the list of loops
		for(std::unique_ptr<Loop> &loop : mLoops) {
			const FlowGraph::Block *block = loop->header;
			const FlowGraph::Block *idom = doms.idom(block);

			// Seach upward through the dominator tree, looking for a block which is
			// the header of a loop
			while(idom != block) {
				block = idom;
				idom = doms.idom(block);

				auto itMap = mLoopMap.find(block);
				if(itMap != mLoopMap.end()) {
					// If this block is the header of a loop, then that loop is the parent of this one
					loop->parent = itMap->second;
					loop->parent->children.insert(loop.get());
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
	const FlowGraph::Block *Loops::findPreheader(Loop &loop)
	{
		const FlowGraph::Block *preheader = 0;

		// Search through the predecessors of the header block
		for(const FlowGraph::Block *block : loop.header->pred) {
			if(loop.blocks.find(block) != loop.blocks.end()) {
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
	void Loops::print(std::ostream &o)
	{
		std::map<Loop *, int> loopMap;
		int num = 1;
		for(std::unique_ptr<Loop> &loop : mLoops) {
			loopMap[loop.get()] = num++;
		}

		for(std::unique_ptr<Loop> &loop : mLoops) {
			o << loopMap[loop.get()] << ": ";
			if(loop->parent != &mRootLoop) {
				o << "parent: " << loopMap[loop->parent] << " | ";
			}
			o << "header: " << ((IR::EntryLabel*)loop->header->entries.front())->name << " | ";
			if(loop->preheader) {
				o << "preheader: " << ((IR::EntryLabel*)loop->preheader->entries.front())->name << " | ";
			}
			o << "blocks: ";
			for(const FlowGraph::Block *block : loop->blocks) {
				o << ((IR::EntryLabel*)block->entries.front())->name << " ";
			}
			o << std::endl;
		}
	}
}