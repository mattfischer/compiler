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
				for(IR::EntryList::iterator itEntry = irBlock->entries.begin(); itEntry != irBlock->entries.end(); itEntry++) {
					IR::Entry *entry = *itEntry;
					irBlock->entries.erase(entry);
					useDefs.remove(entry);
					reachingDefs.remove(entry);
				}
			}
		}

		for(unsigned int i=0; i<procedure->blocks().size(); i++) {
			IR::Block *block = procedure->blocks()[i];
			for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
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
								block->entries.erase(entry);
								useDefs.remove(entry);
							}
							break;
						}
				}
			}
		}
	}
}
