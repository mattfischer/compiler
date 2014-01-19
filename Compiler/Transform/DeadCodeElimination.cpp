#include "Transform/DeadCodeElimination.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/UseDefs.h"
#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"

namespace Transform {
	bool DeadCodeElimination::transform(IR::Procedure *procedure)
	{
		bool changed = false;

		// Construct a flow graph and use-def chains for the procedure
		Analysis::FlowGraph flowGraph(procedure);
		Analysis::UseDefs useDefs(procedure);

		// Iterate through the blocks of the graph
		for(Analysis::FlowGraph::BlockSet::iterator it = flowGraph.blocks().begin(); it != flowGraph.blocks().end(); it++) {
			Analysis::FlowGraph::Block *block = *it;

			// If no control path leads to the block, it can be removed from the graph
			if(block->pred.size() == 0 && block != flowGraph.start()) {
				IR::EntryList::iterator itNext;
				for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry = itNext) {
					itNext = itEntry;
					itNext++;
					IR::Entry *entry = *itEntry;
					if(entry->type == IR::Entry::TypeLabel) {
						continue;
					}

					procedure->entries().erase(entry);
					useDefs.remove(entry);
					delete entry;
				}
				changed = true;
			}
		}

		// Iterate backwards through the procedure's entries
		IR::EntryList::reverse_iterator itNext;
		for(IR::EntryList::reverse_iterator itEntry = procedure->entries().rbegin(); itEntry != procedure->entries().rend(); itEntry = itNext) {
			itNext = itEntry;
			itNext++;
			IR::Entry *entry = *itEntry;
			switch(entry->type) {
				case IR::Entry::TypeMove:
					{
						// If the move's LHS and RHS are the same, the move is unnecessary
						IR::EntryThreeAddr *load = (IR::EntryThreeAddr*)entry;
						if(load->lhs == load->rhs1) {
							procedure->entries().erase(entry);
							useDefs.remove(entry);
							delete entry;
							break;
						}
					}
					// Fall-through
				case IR::Entry::TypeAdd:
				case IR::Entry::TypeAddImm:
				case IR::Entry::TypeMult:
				case IR::Entry::TypeLoadImm:
				case IR::Entry::TypeEqual:
				case IR::Entry::TypeNequal:
				case IR::Entry::TypeLoadRet:
				case IR::Entry::TypeLoadArg:
					{
						// If an assignment has no uses, it is unnecessary
						const IR::EntrySet &uses = useDefs.uses(entry);
						if(uses.empty()) {
							procedure->entries().erase(entry);
							useDefs.remove(entry);
							delete entry;
							changed = true;
						}
						break;
					}
				case IR::Entry::TypeJump:
					{
						// If a jump's target is the next instruction in the procedure, it is unnecessary
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						for(IR::EntryList::iterator itLabel = ++(procedure->entries().find(entry)); itLabel != procedure->entries().end(); itLabel++) {
							IR::Entry *label = *itLabel;
							if(label->type != IR::Entry::TypeLabel) {
								break;
							}

							if(jump->target == label) {
								procedure->entries().erase(jump);
								useDefs.remove(entry);
								delete entry;
								changed = true;
								break;
							}
						}
						break;
					}
			}
		}

		// Count the number of assignments to each symbol in the procedure
		std::map<IR::Symbol*, int> symbolCount;
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			IR::Symbol *assign = entry->assign();
			if(assign) {
				symbolCount[assign]++;
			}
		}

		// Iterate through the procedure's symbols
		IR::SymbolList::iterator itSymbolNext;
		for(IR::SymbolList::iterator itSymbol = procedure->symbols().begin(); itSymbol != procedure->symbols().end(); itSymbol = itSymbolNext) {
			IR::Symbol *symbol = *itSymbol;
			itSymbolNext = itSymbol;
			itSymbolNext++;

			if(symbolCount[symbol] == 0) {
				// If there are no assignments to the symbol, it can be removed from the procedure
				procedure->symbols().erase(itSymbol);
				changed = true;
			}
		}

		return changed;
	}

	/*!
	 * \brief Singleton
	 * \return Instance
	 */
	DeadCodeElimination *DeadCodeElimination::instance()
	{
		static DeadCodeElimination inst;
		return &inst;
	}
}
