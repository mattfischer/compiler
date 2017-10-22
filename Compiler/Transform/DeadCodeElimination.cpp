#include "Transform/DeadCodeElimination.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/UseDefs.h"
#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"

namespace Transform {
	bool DeadCodeElimination::transform(IR::Procedure &procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;

		// Construct a flow graph and use-def chains for the procedure
		const Analysis::FlowGraph &flowGraph = analysis.flowGraph();
		const Analysis::UseDefs &useDefs = analysis.useDefs();
		std::set<IR::Entry*> deleted;

		// Iterate through the blocks of the graph
		for(const std::unique_ptr<Analysis::FlowGraph::Block> &block : flowGraph.blocks()) {
			// If no control path leads to the block, it can be removed from the graph
			if(block->pred.size() == 0 && block.get() != flowGraph.start()) {
				for(IR::Entry *entry : procedure.entries()) {
					if(entry->type == IR::Entry::Type::Label) {
						continue;
					}

					deleted.insert(entry);
					analysis.remove(entry);
				}
				changed = true;
			}
		}

		for (IR::Entry *entry : deleted) {
			procedure.entries().erase(entry);
			delete entry;
		}
		deleted.clear();

		// Iterate backwards through the procedure's entries
		IR::EntryList::reverse_iterator itNext;
		for(IR::Entry *entry : procedure.entries().reversed()) {
			switch(entry->type) {
				case IR::Entry::Type::Move:
					{
						// If the move's LHS and RHS are the same, the move is unnecessary
						IR::EntryThreeAddr *load = (IR::EntryThreeAddr*)entry;
						if(load->lhs == load->rhs1) {
							deleted.insert(entry);
							analysis.remove(entry);
							break;
						}
					}
					// Fall-through
				case IR::Entry::Type::Add:
				case IR::Entry::Type::Mult:
				case IR::Entry::Type::Equal:
				case IR::Entry::Type::Nequal:
				case IR::Entry::Type::LoadRet:
				case IR::Entry::Type::LoadArg:
				case IR::Entry::Type::LoadString:
					{
						// If an assignment has no uses, it is unnecessary
						const std::set<const IR::Entry*> &uses = useDefs.uses(entry);
						if(uses.empty()) {
							deleted.insert(entry);
							analysis.remove(entry);
							changed = true;
						}
						break;
					}
				case IR::Entry::Type::Jump:
					{
						// If a jump's target is the next instruction in the procedure, it is unnecessary
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						for(IR::EntryList::iterator itLabel = ++(procedure.entries().find(entry)); itLabel != procedure.entries().end(); itLabel++) {
							IR::Entry *label = *itLabel;
							if(label->type != IR::Entry::Type::Label) {
								break;
							}

							if(jump->target == label) {
								deleted.insert(entry);
								analysis.remove(entry);
								changed = true;
								break;
							}
						}
						break;
					}
			}
		}

		for (IR::Entry *entry : deleted) {
			procedure.entries().erase(entry);
			delete entry;
		}
		deleted.clear();

		// Count the number of assignments to each symbol in the procedure
		std::map<IR::Symbol*, int> symbolCount;
		for(IR::Entry *entry : procedure.entries()) {
			IR::Symbol *assign = entry->assign();
			if(assign) {
				symbolCount[assign]++;
			}
		}

		// Iterate through the procedure's symbols
		std::vector<std::unique_ptr<IR::Symbol>>::iterator itSymbolNext;
		for(auto itSymbol = procedure.symbols().begin(); itSymbol != procedure.symbols().end(); itSymbol = itSymbolNext) {
			std::unique_ptr<IR::Symbol> &symbol = *itSymbol;
			itSymbolNext = itSymbol;
			itSymbolNext++;

			if(symbolCount[symbol.get()] == 0) {
				// If there are no assignments to the symbol, it can be removed from the procedure
				procedure.symbols().erase(itSymbol);
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
