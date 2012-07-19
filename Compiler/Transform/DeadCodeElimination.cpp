#include "Transform/DeadCodeElimination.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/Analysis.h"
#include "Analysis/UseDefs.h"
#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"

namespace Transform {
	bool DeadCodeElimination::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;

		for(Analysis::FlowGraph::BlockSet::iterator it = analysis.flowGraph().blocks().begin(); it != analysis.flowGraph().blocks().end(); it++) {
			Analysis::FlowGraph::Block *block = *it;
			if(block->pred.size() == 0 && block != analysis.flowGraph().start()) {
				for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
					IR::Entry *entry = *itEntry;
					if(entry->type == IR::Entry::TypeLabel) {
						continue;
					}

					procedure->entries().erase(entry);
					analysis.remove(entry);
				}
				changed = true;
			}
		}

		IR::EntryList::iterator itNext;
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry = itNext) {
			itNext = itEntry;
			itNext++;
			IR::Entry *entry = *itEntry;
			switch(entry->type) {
				case IR::Entry::TypeAdd:
				case IR::Entry::TypeMult:
				case IR::Entry::TypeLoad:
				case IR::Entry::TypeLoadImm:
				case IR::Entry::TypeEqual:
				case IR::Entry::TypeNequal:
					{
						const IR::EntrySet &uses = analysis.useDefs().uses(entry);
						if(uses.empty()) {
							procedure->entries().erase(entry);
							analysis.remove(entry);
							changed = true;
						}
						break;
					}
				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						for(IR::EntryList::iterator itLabel = itNext; itLabel != procedure->entries().end(); itLabel++) {
							IR::Entry *label = *itLabel;
							if(label->type != IR::Entry::TypeLabel) {
								break;
							}

							if(jump->target == label) {
								procedure->entries().erase(jump);
								analysis.remove(jump);
								changed = true;
								break;
							}
						}
						break;
					}
			}
		}

		std::map<IR::Symbol*, int> symbolCount;
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			IR::Symbol *assign = entry->assign();
			if(assign) {
				symbolCount[assign]++;
			}
		}

		IR::Procedure::SymbolList::iterator itSymbolNext;
		for(IR::Procedure::SymbolList::iterator itSymbol = procedure->symbols().begin(); itSymbol != procedure->symbols().end(); itSymbol = itSymbolNext) {
			IR::Symbol *symbol = *itSymbol;
			itSymbolNext = itSymbol;
			itSymbolNext++;

			if(symbolCount[symbol] == 0) {
				procedure->symbols().erase(itSymbol);
				changed = true;
			}
		}

		return changed;
	}

	DeadCodeElimination *DeadCodeElimination::instance()
	{
		static DeadCodeElimination inst;
		return &inst;
	}
}
