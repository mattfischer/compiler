#include "OptPassIntoSSA.h"

#include "Analysis/Dominance.h"

#include <queue>
#include <sstream>
#include <vector>
#include <set>

static std::string newSymbolName(IR::Symbol *base, int version)
{
	std::stringstream ss;

	ss << base->name << "." << version;
	return ss.str();
}

bool OptPassIntoSSA::optimizeProcedure(IR::Procedure *proc)
{
	std::vector<IR::Symbol*> newSymbols;

	Analysis::DominatorTree domTree(proc->blocks());
	Analysis::DominanceFrontiers domFrontiers(domTree);

	for(unsigned int i=0; i<proc->symbols().size(); i++) {
		IR::Symbol *symbol = proc->symbols()[i];
		std::queue<IR::Block*> blocks;

		// Initialize queue with variable assignments
		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];

			for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
				if(entry->assigns(symbol)) {
					blocks.push(block);
				}
			}
		}

		// Insert Phi functions
		while(!blocks.empty()) {
			IR::Block *block = blocks.front();
			blocks.pop();

			const std::set<IR::Block*> &frontiers = domFrontiers.frontiers(block);
			for(std::set<IR::Block*>::const_iterator it = frontiers.begin(); it != frontiers.end(); it++) {
				IR::Block *frontier = *it;
				IR::Entry *head = frontier->head()->next;
				
				if(head->type != IR::Entry::TypePhi || ((IR::EntryPhi*)head)->lhs != symbol) {
					frontier->prependEntry(new IR::EntryPhi(symbol, symbol, (int)frontier->pred.size()));
					blocks.push(frontier);
				}
			}
		}

		// Rename variables
		int nextVersion = 0;
		std::map<IR::Block*, IR::Symbol*> activeList;
		activeList[proc->start()] = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->type);
		newSymbols.push_back(activeList[proc->start()]);
		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];
			if(activeList.find(block) == activeList.end())
				activeList[block] = activeList[domTree.idom(block)];
			IR::Symbol *active = activeList[block];

			for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
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
			for(unsigned int k=0; k<block->succ.size(); k++) {
				IR::Block *succ = block->succ[k];
				IR::Entry *head = succ->head()->next;

				if(head->type == IR::Entry::TypePhi && ((IR::EntryPhi*)head)->base == symbol) {
					for(unsigned int l=0; l<succ->pred.size(); l++) {
						if(succ->pred[l] == block) {
							((IR::EntryPhi*)head)->setArg(l, active);
							break;
						}
					}
				}
			}
		}
	}

	// Add newly-created symbols into symbol table
	for(unsigned int i=0; i<newSymbols.size(); i++) {
		proc->addSymbol(newSymbols[i]);
	}

	return false;
}