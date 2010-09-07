#include "OptPassIntoSSA.h"

#include <queue>
#include <sstream>

static std::string newSymbolName(IR::Symbol *base, int version)
{
	std::stringstream ss;

	ss << base->name << "." << version;
	return ss.str();
}

void OptPassIntoSSA::optimizeProcedure(IR::Procedure *proc)
{
	std::vector<IR::Symbol*> newSymbols;

	proc->computeDominance();

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

			for(unsigned int j=0; j<block->domFrontiers.size(); j++) {
				IR::Block *frontier = block->domFrontiers[j];
				IR::Entry *head = frontier->head()->next;
				
				if(head->type != IR::Entry::TypePhi || ((IR::EntryPhi*)head)->lhs != symbol) {
					frontier->prependEntry(new IR::EntryPhi(symbol, symbol, (int)frontier->pred.size()));
					blocks.push(frontier);
				}
			}
		}

		// Rename variables
		int nextVersion = 0;
		std::vector<IR::Symbol*> activeList(proc->blocks().size());
		activeList[0] = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->type);
		newSymbols.push_back(activeList[0]);
		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];
			if(!activeList[j])
				activeList[j] = activeList[block->idom->number];
			IR::Symbol *active = activeList[j];

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
					activeList[j] = active;
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
}