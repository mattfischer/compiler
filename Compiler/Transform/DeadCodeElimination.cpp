#include "Transform/DeadCodeElimination.h"

#include "IR/Procedure.h"
#include "IR/Block.h"
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
				IR::Block *irBlock = block->irBlock;
				for(IR::Entry *entry = irBlock->head()->next; entry != irBlock->tail(); entry = entry->next) {
					entry->remove();
					useDefs.remove(entry);
					reachingDefs.remove(entry);
				}
			}
		}

		for(unsigned int i=0; i<procedure->blocks().size(); i++) {
			IR::Block *block = procedure->blocks()[i];
			for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
				switch(entry->type) {
					case IR::Entry::TypeAdd:
					case IR::Entry::TypeMult:
					case IR::Entry::TypeLoad:
					case IR::Entry::TypeLoadImm:
					case IR::Entry::TypeEqual:
					case IR::Entry::TypeNequal:
						{
							const Analysis::UseDefs::EntrySet &uses = useDefs.uses(entry);
							if(uses.empty()) {
								entry->remove();
								useDefs.remove(entry);
							}
							break;
						}
				}
			}
		}
	}
}
