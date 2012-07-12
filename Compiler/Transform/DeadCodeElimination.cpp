#include "Transform/DeadCodeElimination.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/UseDefs.h"
#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"

namespace Transform {
	void DeadCodeElimination::transform(IR::Procedure *procedure, Analysis::UseDefs &useDefs, Analysis::ReachingDefs &reachingDefs, Analysis::FlowGraph &flowGraph)
	{
		for(Analysis::FlowGraph::BlockSet::iterator it = flowGraph.blocks().begin(); it != flowGraph.blocks().end(); it++) {
			Analysis::FlowGraph::Block *block = *it;
			if(block->pred.size() == 0 && block != flowGraph.start()) {
				for(IR::EntryList::iterator itEntry = procedure->entries().find(block->label); itEntry != procedure->entries().find(block->end); itEntry++) {
					IR::Entry *entry = *itEntry;
					procedure->entries().erase(entry);
					useDefs.remove(entry);
					reachingDefs.remove(entry);
				}
			}
		}

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			switch(entry->type) {
				case IR::Entry::TypeAdd:
				case IR::Entry::TypeMult:
				case IR::Entry::TypeLoad:
				case IR::Entry::TypeLoadImm:
				case IR::Entry::TypeEqual:
				case IR::Entry::TypeNequal:
					{
						const IR::EntrySet &uses = useDefs.uses(entry);
						if(uses.empty()) {
							procedure->entries().erase(entry);
							useDefs.remove(entry);
						}
						break;
					}
			}
		}
	}
}
