#include "Transform/CopyProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"

#include "Analysis/DataFlow.h"

namespace Transform {
	bool CopyProp::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;
		//changed |= backward(procedure, analysis);
		changed |= forward(procedure, analysis);

		return changed;
	}

	/*!
	 * \brief Propagate copies forward through the procedure
	 * \param procedure Procedure to transform
	 * \return Whether the procedure was changed
	 */
	bool CopyProp::forward(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;
		IR::EntrySet allLoads;
		std::map<IR::Entry*, IR::EntrySet> gen;
		std::map<IR::Entry*, IR::EntrySet> kill;

		// Construct the set of all move entries in the procedure
		for(IR::Entry *entry : procedure->entries()) {
			if(entry->type == IR::Entry::Type::Move && ((IR::EntryThreeAddr*)entry)->rhs1) {
				allLoads.insert(entry);

				// A move instruction generates that entry
				gen[entry].insert(entry);
			}
		}

		// Construct gen/kill sets for data flow analysis
		for(IR::Entry *entry : procedure->entries()) {
			if(!entry->assign()) {
				continue;
			}

			for(IR::Entry *load : allLoads) {
				// Any entry which assigns to a symbol kills all moves to that same symbol
				if(entry != load && (load->assign() == entry->assign() || load->uses(entry->assign()))) {
					kill[entry].insert(load);
				}
			}
		}

		// Perform forward data flow analysis with the gen/kill sets constructed above
		Analysis::DataFlow<IR::Entry*> dataFlow;
		Analysis::FlowGraph &flowGraph = analysis.flowGraph();
		std::map<IR::Entry*, IR::EntrySet> loads = dataFlow.analyze(flowGraph, gen, kill, allLoads, Analysis::DataFlow<IR::Entry*>::Meet::Intersect, Analysis::DataFlow<IR::Entry*>::Direction::Forward);

		// Iterate through the procedure's entries
		for(IR::Entry *entry : procedure->entries()) {
			// If a move entry survived to this point, any use of the move's LHS can be
			// replaced with its RHS
			for(IR::Entry *load : loads[entry]) {
				IR::EntryThreeAddr *loadEntry = (IR::EntryThreeAddr*)load;
				if(entry->uses(loadEntry->lhs)) {
					analysis.replaceUse(entry, loadEntry->lhs, loadEntry->rhs1);
					entry->replaceUse(loadEntry->lhs, loadEntry->rhs1);
					changed = true;
				}
			}
		}

		return changed;
	}

	/*!
	 * \brief Perform backward copy propagation through the procedure
	 * \param procedure Procedure to transform
	 * \return Whether the procedure was changed
	 */
	bool CopyProp::backward(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;
		IR::EntrySet allLoads;
		std::map<IR::Entry*, IR::EntrySet> gen;
		std::map<IR::Entry*, IR::EntrySet> kill;

		// Construct the set of all move entries in the procedure
		for(IR::Entry *entry : procedure->entries()) {
			if(entry->type == IR::Entry::Type::Move && ((IR::EntryThreeAddr*)entry)->rhs1) {
				allLoads.insert(entry);

				// A move entry generates that entry
				gen[entry].insert(entry);
			}
		}

		// Construct gen/kill sets for data flow analysis
		for(IR::Entry *entry : procedure->entries()) {
			for(IR::Entry *load : allLoads) {
				IR::EntryThreeAddr *loadThreeAddr = (IR::EntryThreeAddr*)load;

				// Any entry which assigns to a symbol kills all moves to that same symbol
				if(entry != load && (entry->assign() == loadThreeAddr->lhs || entry->assign() == loadThreeAddr->rhs1 || entry->uses(loadThreeAddr->lhs) || entry->uses(loadThreeAddr->rhs1))) {
					kill[entry].insert(load);
				}
			}
		}

		// Perform backwards data flow analysis with the gen/kill sets constructed above
		Analysis::DataFlow<IR::Entry*> dataFlow;
		Analysis::FlowGraph &flowGraph = analysis.flowGraph();
		std::map<IR::Entry*, IR::EntrySet> loads = dataFlow.analyze(flowGraph, gen, kill, allLoads, Analysis::DataFlow<IR::Entry*>::Meet::Intersect, Analysis::DataFlow<IR::Entry*>::Direction::Backward);

		IR::EntrySet deleted;

		// Iterate backwards through the procedure's entries
		for(IR::Entry *entry : procedure->entries()) {
			for(IR::Entry *load : loads[entry]) {
				IR::EntryThreeAddr *loadThreeAddr = (IR::EntryThreeAddr*)load;

				// If a move entry survived to this point, any assignment to the load's RHS
				// can be assigned directly to its LHS instead
				if(entry->assign() == loadThreeAddr->rhs1) {
					entry->replaceAssign(entry->assign(), loadThreeAddr->lhs);
					deleted.insert(load);
					changed = true;
				}
			}
		}

		// Reflect the change in variables by changing all loads into no-ops, since their RHS
		// no longer contains the correct value
		for(IR::Entry *load : deleted) {
			IR::EntryThreeAddr *loadThreeAddr = (IR::EntryThreeAddr*)load;
			loadThreeAddr->rhs1 = loadThreeAddr->lhs;
		}

		if(changed) {
			analysis.invalidate();
		}

		return changed;
	}

	/*!
	 * \brief Singleton
	 * \return Instance
	 */
	CopyProp *CopyProp::instance()
	{
		static CopyProp inst;
		return &inst;
	}
}