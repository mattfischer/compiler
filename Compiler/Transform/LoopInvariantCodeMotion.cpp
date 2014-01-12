#include "Transform/LoopInvariantCodeMotion.h"

#include "Analysis/Loops.h"
#include "Analysis/FlowGraph.h"

#include "IR/Procedure.h"

namespace Transform {
	bool LoopInvariantCodeMotion::transform(IR::Procedure *procedure)
	{
		bool changed = false;
		Analysis::Loops loops(procedure);

		processLoop(loops.rootLoop(), procedure, loops);
		return changed;
	}

	void LoopInvariantCodeMotion::processLoop(Analysis::Loops::Loop *loop, IR::Procedure *procedure, Analysis::Loops &loops)
	{
		for(Analysis::Loops::LoopSet::iterator itLoop = loop->children.begin(); itLoop != loop->children.end(); itLoop++) {
			Analysis::Loops::Loop *child = *itLoop;
			processLoop(child, procedure, loops);
		}

		if(loop == loops.rootLoop()) {
			return;
		}

		IR::EntrySet invariants;
		typedef std::map<IR::Symbol *, IR::EntrySet> SymbolToEntrySetMap;
		SymbolToEntrySetMap defs;

		for(Analysis::FlowGraph::BlockSet::iterator itBlock = loop->blocks.begin(); itBlock != loop->blocks.end(); itBlock++) {
			Analysis::FlowGraph::Block *block = *itBlock;
			for(IR::EntrySubList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				if(entry->assign()) {
					defs[entry->assign()].insert(entry);
				}

				// TODO: Non-constant invariants
				if(entry->type == IR::Entry::TypeLoadImm) {
					invariants.insert(entry);
				}
			}
		}

		for(IR::EntrySet::iterator itEntry = invariants.begin(); itEntry != invariants.end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			// TODO: Test for use outside of loops
			if(defs[entry->assign()].size() != 1) {
				continue;
			}

			procedure->entries().erase(entry);
			procedure->entries().insert(*loop->preheader->entries.end(), entry);
		}
	}

	LoopInvariantCodeMotion *LoopInvariantCodeMotion::instance()
	{
		static LoopInvariantCodeMotion inst;
		return &inst;
	}
}