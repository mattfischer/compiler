#include "Transform/SSA.h"

#include "Analysis/DominatorTree.h"
#include "Analysis/DominanceFrontiers.h"
#include "Analysis/FlowGraph.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Util/UniqueQueue.h"

#include <sstream>
#include <vector>
#include <set>

namespace Transform {
	/*!
	 * \brief Construct a new symbol name out of a base symbol
	 * \param base Base symbol
	 * \param version Version number to give to the symbol
	 * \return New symbol name
	 */
	std::string SSA::newSymbolName(IR::Symbol *base, int version)
	{
		std::stringstream ss;

		ss << base->name << "." << version;
		return ss.str();
	}

	bool SSA::transform(IR::Procedure *proc, Analysis::Analysis &analysis)
	{
		std::vector<IR::Symbol*> newSymbols;

		// Perform flow graph and dominance analysis on the procedure
		Analysis::FlowGraph *flowGraph = analysis.flowGraph();
		Analysis::DominatorTree dominatorTree(proc, flowGraph);
		Analysis::DominanceFrontiers dominanceFrontiers(dominatorTree);

		// Iterate through the list of symbols in the procedure
		for(IR::Symbol *symbol : proc->symbols()) {
			Util::UniqueQueue<Analysis::FlowGraph::Block*> blocks;

			// Initialize queue with variable assignments
			for(Analysis::FlowGraph::Block *block : flowGraph->blocks()) {
				for(IR::Entry *entry : block->entries) {
					if(entry->assign() == symbol) {
						blocks.push(block);
					}
				}
			}

			// Insert Phi functions at each dominance frontier for the block
			while(!blocks.empty()) {
				Analysis::FlowGraph::Block *block = blocks.front();
				blocks.pop();

				for(Analysis::FlowGraph::Block *frontier : dominanceFrontiers.frontiers(block)) {
					IR::Entry *head = *(frontier->entries.begin()++);

					if(head->type != IR::Entry::TypePhi || ((IR::EntryPhi*)head)->lhs != symbol) {
						proc->entries().insert(head, new IR::EntryPhi(symbol, symbol, (int)frontier->pred.size()));
						blocks.push(frontier);
					}
				}
			}

			// Rename variables
			int nextVersion = 0;
			std::map<Analysis::FlowGraph::Block*, IR::Symbol*> activeList;
			activeList[flowGraph->start()] = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->size, symbol->symbol);
			newSymbols.push_back(activeList[flowGraph->start()]);
			for(Analysis::FlowGraph::Block *block : flowGraph->blocks()) {
				if(activeList.find(block) == activeList.end())
					activeList[block] = activeList[dominatorTree.idom(block)];
				IR::Symbol *active = activeList[block];

				for(IR::Entry *entry : block->entries) {
					if(entry->uses(symbol)) {
						entry->replaceUse(symbol, active);
					}

					// Create a new version of the variable for each assignment
					if(entry->assign() == symbol) {
						IR::Symbol *newSymbol = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->size, symbol->symbol);
						newSymbols.push_back(newSymbol);
						entry->replaceAssign(active, newSymbol);
						active = newSymbol;
						activeList[block] = active;
					}
				}

				// Propagate variable uses into Phi functions
				for(Analysis::FlowGraph::Block *succ : block->succ) {
					IR::Entry *head = *(succ->entries.begin()++);

					if(head->type == IR::Entry::TypePhi && ((IR::EntryPhi*)head)->base == symbol) {
						int l = 0;
						for(Analysis::FlowGraph::Block *pred : succ->pred) {
							if(pred == block) {
								((IR::EntryPhi*)head)->setArg(l, active);
								break;
							}
							l++;
						}
					}
				}
			}
		}

		// Add newly-created symbols into symbol table
		for(IR::Symbol *symbol : newSymbols) {
			proc->addSymbol(symbol);
		}

		analysis.invalidate();

		return true;
	}

	/*!
	 * \brief Singleton
	 * \return Instance
	 */
	SSA *SSA::instance()
	{
		static SSA inst;
		return &inst;
	}
}