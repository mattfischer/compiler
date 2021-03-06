#include "Transform/LoopInvariantCodeMotion.h"

#include "Analysis/Loops.h"
#include "Analysis/FlowGraph.h"

#include "IR/Procedure.h"

namespace Transform {
	bool LoopInvariantCodeMotion::transform(IR::Procedure &procedure, Analysis::Analysis &analysis)
	{
		// Perform loop analysis on the procedure
		Analysis::Loops loops(procedure, analysis.flowGraph());

		// Recursively process the root loop of the procedure
		bool changed = processLoop(*loops.rootLoop(), procedure, loops);

		if(changed) {
			analysis.invalidate();
		}

		return changed;
	}

	/*!
	 * \brief Recursively analyze and transform a loop and its descendents
	 * \param loop Loop to transform
	 * \param procedure Procedure that contains the loop
	 * \param loops Loop analysis of the procedure
	 */
	bool LoopInvariantCodeMotion::processLoop(Analysis::Loops::Loop &loop, IR::Procedure &procedure, Analysis::Loops &loops)
	{
		bool changed = false;

		// Process all child loops recursively
		for(Analysis::Loops::Loop *child : loop.children) {
			changed |= processLoop(*child, procedure, loops);
		}

		// There is no point in processing the root loop, since there is nowhere to move code to
		if(&loop == loops.rootLoop()) {
			return changed;
		}

		// Construct a set of entries which are invariant in the loop
		std::set<const IR::Entry*> invariants;
		std::map<const IR::Symbol *, std::set<const IR::Entry*>> defs;

		for(const Analysis::FlowGraph::Block *block : loop.blocks) {
			for(const IR::Entry *entry : block->entries) {
				if(entry->assign()) {
					// Record all definitions which take place inside of the loop
					defs[entry->assign()].insert(entry);
				}

				// TODO: Non-constant invariants
				if(entry->type == IR::Entry::Type::Move && ((IR::EntryThreeAddr*)entry)->rhs1 == 0) {
					// If a symbol is assigned to a constant, it is invariant
					invariants.insert(entry);
				}
			}
		}

		// Iterate through the invariant entries discovered above
		for(const IR::Entry *constEntry : invariants) {
			IR::Entry *entry = procedure.entries().entry(constEntry);
			// TODO: Test for use outside of loops
			if(defs[entry->assign()].size() != 1) {
				continue;
			}

			// Move the entry into the loop's preheader
			procedure.entries().erase(entry);
			procedure.entries().insert(*loop.preheader->entries.end(), entry);
			changed = true;
		}

		return changed;
	}

	/*!
	 * \brief Singleton
	 * \return Instance
	 */
	LoopInvariantCodeMotion *LoopInvariantCodeMotion::instance()
	{
		static LoopInvariantCodeMotion inst;
		return &inst;
	}
}