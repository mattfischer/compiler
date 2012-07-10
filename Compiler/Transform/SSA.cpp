#include "Transform/SSA.h"

#include "Analysis/Dominance.h"
#include "Analysis/FlowGraph.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Block.h"

#include <queue>
#include <sstream>
#include <vector>
#include <set>

namespace Transform {
	std::string SSA::newSymbolName(IR::Symbol *base, int version)
	{
		std::stringstream ss;

		ss << base->name << "." << version;
		return ss.str();
	}

	void SSA::transform(IR::Procedure *proc, Analysis::FlowGraph &flowGraph)
	{
		std::vector<IR::Symbol*> newSymbols;

		Analysis::DominatorTree domTree(flowGraph);
		Analysis::DominanceFrontiers domFrontiers(domTree);

		for(IR::Procedure::SymbolList::iterator symIt = proc->symbols().begin(); symIt != proc->symbols().end(); symIt++) {
			IR::Symbol *symbol = *symIt;
			std::queue<Analysis::FlowGraph::Block*> blocks;

			// Initialize queue with variable assignments
			for(Analysis::FlowGraph::BlockSet::iterator blockIt = flowGraph.blocks().begin(); blockIt != flowGraph.blocks().end(); blockIt++) {
				Analysis::FlowGraph::Block *block = *blockIt;
				IR::Block *irBlock = block->irBlock;
				for(IR::Entry *entry = irBlock->head()->next; entry != irBlock->tail(); entry = entry->next) {
					if(entry->assigns(symbol)) {
						blocks.push(block);
					}
				}
			}

			// Insert Phi functions
			while(!blocks.empty()) {
				Analysis::FlowGraph::Block *block = blocks.front();
				blocks.pop();

				const Analysis::DominanceFrontiers::BlockSet &frontiers = domFrontiers.frontiers(block);
				for(Analysis::DominanceFrontiers::BlockSet::const_iterator frontIt = frontiers.begin(); frontIt != frontiers.end(); frontIt++) {
					Analysis::FlowGraph::Block *frontier = *frontIt;
					IR::Block *irFrontier = frontier->irBlock;
					IR::Entry *head = irFrontier->head()->next;

					if(head->type != IR::Entry::TypePhi || ((IR::EntryPhi*)head)->lhs != symbol) {
						irFrontier->prependEntry(new IR::EntryPhi(symbol, symbol, (int)frontier->pred.size()));
						blocks.push(frontier);
					}
				}
			}

			// Rename variables
			int nextVersion = 0;
			std::map<Analysis::FlowGraph::Block*, IR::Symbol*> activeList;
			activeList[flowGraph.start()] = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->type);
			newSymbols.push_back(activeList[flowGraph.start()]);
			for(Analysis::FlowGraph::BlockSet::iterator blockIt = flowGraph.blocks().begin(); blockIt != flowGraph.blocks().end(); blockIt++) {
				Analysis::FlowGraph::Block *block = *blockIt;
				IR::Block *irBlock = block->irBlock;
				if(activeList.find(block) == activeList.end())
					activeList[block] = activeList[domTree.idom(block)];
				IR::Symbol *active = activeList[block];

				for(IR::Entry *entry = irBlock->head()->next; entry != irBlock->tail(); entry = entry->next) {
					if(entry->uses(symbol)) {
						entry->replaceUse(symbol, active);
					}

					// Create a new version of the variable for each assignment
					if(entry->assigns(symbol)) {
						IR::Symbol *newSymbol = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->type);
						newSymbols.push_back(newSymbol);
						entry->replaceAssign(active, newSymbol);
						active = newSymbol;
						activeList[block] = active;
					}
				}

				// Propagate variable uses into Phi functions
				for(Analysis::FlowGraph::Block::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
					Analysis::FlowGraph::Block *succ = *it;
					IR::Block *irSucc = succ->irBlock;
					IR::Entry *head = irSucc->head()->next;

					if(head->type == IR::Entry::TypePhi && ((IR::EntryPhi*)head)->base == symbol) {
						int l = 0;
						for(Analysis::FlowGraph::Block::BlockSet::iterator it2 = succ->pred.begin(); it2 != succ->pred.end(); it2++) {
							Analysis::FlowGraph::Block *pred = *it2;
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
		for(unsigned int i=0; i<newSymbols.size(); i++) {
			proc->addSymbol(newSymbols[i]);
		}
	}
}