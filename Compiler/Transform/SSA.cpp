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

	bool SSA::transform(IR::Procedure &proc, Analysis::Analysis &analysis)
	{
		std::vector<std::unique_ptr<IR::Symbol>> newSymbols;

		// Perform flow graph and dominance analysis on the procedure
		Analysis::FlowGraph &flowGraph = analysis.flowGraph();
		Analysis::DominatorTree dominatorTree(proc, flowGraph);
		Analysis::DominanceFrontiers dominanceFrontiers(dominatorTree);

		// Iterate through the list of symbols in the procedure
		for(std::unique_ptr<IR::Symbol> &symbol : proc.symbols()) {
			Util::UniqueQueue<Analysis::FlowGraph::Block*> blocks;

			// Initialize queue with variable assignments
			for(std::unique_ptr<Analysis::FlowGraph::Block> &block : flowGraph.blocks()) {
				for(IR::Entry *entry : block->entries) {
					if(entry->assign() == symbol.get()) {
						blocks.push(block.get());
					}
				}
			}

			// Insert Phi functions at each dominance frontier for the block
			while(!blocks.empty()) {
				Analysis::FlowGraph::Block *block = blocks.front();
				blocks.pop();

				for(Analysis::FlowGraph::Block *frontier : dominanceFrontiers.frontiers(block)) {
					IR::Entry *head = *(frontier->entries.begin()++);

					if(head->type != IR::Entry::Type::Phi || ((IR::EntryPhi*)head)->lhs != symbol.get()) {
						proc.entries().insert(head, new IR::EntryPhi(symbol.get(), symbol.get(), (int)frontier->pred.size()));
						blocks.push(frontier);
					}
				}
			}

			// Rename variables
			int nextVersion = 0;
			std::map<Analysis::FlowGraph::Block*, IR::Symbol*> activeList;
			std::unique_ptr<IR::Symbol> newSymbol = std::make_unique<IR::Symbol>(newSymbolName(symbol.get(), nextVersion++), symbol->size, symbol->symbol);
			activeList[flowGraph.start()] = newSymbol.get();
			newSymbols.push_back(std::move(newSymbol));
			for(std::unique_ptr<Analysis::FlowGraph::Block> &block : flowGraph.blocks()) {
				if(activeList.find(block.get()) == activeList.end())
					activeList[block.get()] = activeList[dominatorTree.idom(block.get())];
				IR::Symbol *active = activeList[block.get()];

				for(IR::Entry *entry : block->entries) {
					if(entry->uses(symbol.get())) {
						entry->replaceUse(symbol.get(), active);
					}

					// Create a new version of the variable for each assignment
					if(entry->assign() == symbol.get()) {
						std::unique_ptr<IR::Symbol> newSymbol = std::make_unique<IR::Symbol>(newSymbolName(symbol.get(), nextVersion++), symbol->size, symbol->symbol);
						entry->replaceAssign(active, newSymbol.get());
						active = newSymbol.get();
						activeList[block.get()] = active;
						newSymbols.push_back(std::move(newSymbol));
					}
				}

				// Propagate variable uses into Phi functions
				for(Analysis::FlowGraph::Block *succ : block->succ) {
					IR::Entry *head = *(succ->entries.begin()++);

					if(head->type == IR::Entry::Type::Phi && ((IR::EntryPhi*)head)->base == symbol.get()) {
						int l = 0;
						for(Analysis::FlowGraph::Block *pred : succ->pred) {
							if(pred == block.get()) {
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
		for(std::unique_ptr<IR::Symbol> &symbol : newSymbols) {
			proc.addSymbol(std::move(symbol));
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