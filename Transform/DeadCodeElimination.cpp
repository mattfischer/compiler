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
		std::set<const IR::Entry*> deleted;

		// Iterate through the blocks of the graph
		for(const std::unique_ptr<Analysis::FlowGraph::Block> &block : flowGraph.blocks()) {
			// If no control path leads to the block, it can be removed from the graph
			if(block->pred.size() == 0 && block.get() != flowGraph.start()) {
				for(const IR::Entry *entry : block->entries) {
					if(entry->type == IR::Entry::Type::Label) {
						continue;
					}

					deleted.insert(entry);
					analysis.remove(entry);
				}
				changed = true;
			}
		}

		for (const IR::Entry *entry : deleted) {
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

		for (const IR::Entry *entry : deleted) {
			procedure.entries().erase(entry);
			delete entry;
		}
		deleted.clear();

		// Count the number of assignments to each symbol in the procedure
		std::map<const IR::Symbol*, int> symbolCount;
		for(IR::Entry *entry : procedure.entries()) {
			const IR::Symbol *assign = entry->assign();
			if(assign) {
				symbolCount[assign]++;
			}
		}

		auto it = std::remove_if(procedure.symbols().begin(), procedure.symbols().end(), [&](auto &symbol) {
			return symbolCount[symbol.get()] == 0;
		});
		if(it != procedure.symbols().end()) {
			changed = true;
			procedure.symbols().erase(it, procedure.symbols().end());
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
