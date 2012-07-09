#include "Transform/SSA.h"

#include "Analysis/Dominance.h"

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

	void SSA::transform(IR::Procedure *proc)
	{
		std::vector<IR::Symbol*> newSymbols;

		Analysis::DominatorTree domTree(proc->blocks());
		Analysis::DominanceFrontiers domFrontiers(domTree);

		for(IR::Procedure::SymbolList::iterator symIt = proc->symbols().begin(); symIt != proc->symbols().end(); symIt++) {
			IR::Symbol *symbol = *symIt;
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

				const Analysis::DominanceFrontiers::BlockSet &frontiers = domFrontiers.frontiers(block);
				for(Analysis::DominanceFrontiers::BlockSet::const_iterator frontIt = frontiers.begin(); frontIt != frontiers.end(); frontIt++) {
					IR::Block *frontier = *frontIt;
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
				for(IR::Block::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
					IR::Block *succ = *it;
					IR::Entry *head = succ->head()->next;

					if(head->type == IR::Entry::TypePhi && ((IR::EntryPhi*)head)->base == symbol) {
						int l = 0;
						for(IR::Block::BlockSet::iterator it2 = succ->pred.begin(); it2 != succ->pred.end(); it2++) {
							IR::Block *pred = *it2;
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