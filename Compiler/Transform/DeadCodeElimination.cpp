#include "Transform/DeadCodeElimination.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/Analysis.h"
#include "Analysis/UseDefs.h"
#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"

namespace Transform {
	void DeadCodeElimination::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		for(Analysis::FlowGraph::BlockSet::iterator it = analysis.flowGraph().blocks().begin(); it != analysis.flowGraph().blocks().end(); it++) {
			Analysis::FlowGraph::Block *block = *it;
			if(block->pred.size() == 0 && block != analysis.flowGraph().start()) {
				for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
					IR::Entry *entry = *itEntry;
					procedure->entries().erase(entry);
					analysis.remove(entry);
				}
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
						}
						break;
					}
				case IR::Entry::TypeJump:
					{
						IR::EntryJump *jump = (IR::EntryJump*)entry;
						if(jump->target == *itNext) {
							procedure->entries().erase(entry);
							analysis.remove(entry);
						}
						break;
					}
			}
		}
	}

	DeadCodeElimination &DeadCodeElimination::instance()
	{
		static DeadCodeElimination inst;
		return inst;
	}
}
