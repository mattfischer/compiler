#include "Back/RegisterAllocator.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/InterferenceGraph.h"
#include "Analysis/LiveVariables.h"
#include "Analysis/Loops.h"
#include "Analysis/UseDefs.h"
#include "Analysis/Constants.h"

#include "Transform/LiveRangeRenaming.h"

#include "Front/Type.h"

#include "Util/Log.h"

#include <vector>
#include <sstream>

namespace Back {

static const int MaxRegisters = 13;
static const int CallerSavedRegisters = 4;

/*!
 * \brief Estimate the cost of spilling each symbol
 * \param procedure Procedure to analyze
 * \return Map from symbol to its spill cost
 */
std::map<IR::Symbol*, int> getSpillCosts(const IR::Procedure &procedure, Analysis::FlowGraph &flowGraph)
{
	std::map<IR::Symbol*, int> costs;

	// Perform flow graph and loop analysis on the procedure
	Analysis::Loops loops(procedure, flowGraph);
	Analysis::Loops::Loop *rootLoop = loops.rootLoop();

	// Iterate through the blocks of the procedure
	for(Analysis::FlowGraph::Block *block : rootLoop->blocks) {
		// Determine the cost of a variable use in this block, based on its loop depth.
		int cost = 1;
		for(std::unique_ptr<Analysis::Loops::Loop> &loop : loops.loops()) {
			if(loop->blocks.find(block) != loop->blocks.end()) {
				cost *= 10;
			}
		}

		// Iterate through the block's entries
		for(IR::Entry *entry : block->entries) {
			// Iterate through the symbols in the procedure
			for(const std::unique_ptr<IR::Symbol> &symbol : procedure.symbols()) {
				// Any assignment to the variable adds to its cost
				if(entry->assign() == symbol.get()) {
					costs[symbol.get()] += cost;
				}

				// Any use of the variable also adds to its cost
				if(entry->uses(symbol.get())) {
					costs[symbol.get()] += cost;
				}
			}
		}
	}

	return costs;
}

/*!
 * \brief Add interferences to a graph between one symbol and a set of symbols
 * \param graph Graph to modify
 * \param symbols Set of symbols to add to graph
 * \param symbol Symbol to add interferences to
 * \param excludeSymbol Symbol to exclude from the above symbol set, for convenience
 */
void addInterferences(Analysis::InterferenceGraph &graph, const std::set<IR::Symbol*> &symbols, IR::Symbol *target, IR::Symbol *exclude)
{
	for(IR::Symbol *symbol : symbols) {
		if(symbol != exclude) {
			graph.addEdge(target, symbol);
		}
	}
}

/*!
 * \brief Add graph interferences to caller-saved registers for variables which are live across procedure calls
 * \param graph Graph to modify
 * \param callerSavedRegisters List of special caller-saved register symbols
 * \param procedure Procedure being analyzed
 * \param liveVariables Live variables in current procedure
 */
void addProcedureCallInterferences(Analysis::InterferenceGraph &graph, const std::vector<IR::Symbol*> callerSavedRegisters, IR::Procedure &procedure, Analysis::LiveVariables &liveVariables)
{
	// Iterate through the procedure's entries
	for(IR::Entry *entry : procedure.entries()) {
		IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;

		// Determine variables which are live in this entry
		const std::set<IR::Symbol*> &variables = liveVariables.variables(entry);

		switch(entry->type) {
			case IR::Entry::Type::Call:
				// A call creates interferences between all caller-saved registers and
				// all live variables
				for(IR::Symbol *reg : callerSavedRegisters) {
					addInterferences(graph, variables, reg, 0);
				}
				break;

			case IR::Entry::Type::LoadRet:
				// A return load creates interferences with the return register for all variables
				// except the load's LHS
				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->lhs);
				break;

			case IR::Entry::Type::StoreRet:
				// A return store creates interferences with the return register for all variables
				// except the store's RHS
				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->rhs1);
				break;

			case IR::Entry::Type::LoadArg:
				// An argument load creates interferences with the argument register for all variables
				// except the load's LHS
				addInterferences(graph, variables, callerSavedRegisters[threeAddr->imm], threeAddr->lhs);
				break;

			case IR::Entry::Type::StoreArg:
				// An argument store creates interferences with the argument register for all variables
				// except the load's RHS
				addInterferences(graph, variables, callerSavedRegisters[threeAddr->imm], threeAddr->rhs1);
				break;
		}
	}
}

/*!
 * \brief Get preferred registers for each symbol in a procedure
 * \param procedure Procedure to analyze
 * \return Map from symbol to preferred register number
 */
std::map<IR::Symbol*, int> getPreferredRegisters(IR::Procedure &procedure)
{
	std::map<IR::Symbol*, int> preferredRegisters;

	// Iterate through the procedure's entries
	for(IR::Entry *entry : procedure.entries()) {
		IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;

		switch(entry->type) {
			case IR::Entry::Type::LoadRet:
				// Set the preferred register for the LHS of a return load to the return register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(threeAddr->lhs) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->lhs] = 0;
				} else {
					preferredRegisters[threeAddr->lhs] = -1;
				}
				break;

			case IR::Entry::Type::StoreRet:
				// Set the preferred register for the RHS of a return store to the return register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(threeAddr->rhs1) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->rhs1] = 0;
				} else {
					preferredRegisters[threeAddr->rhs1] = -1;
				}
				break;

			case IR::Entry::Type::LoadArg:
				// Set the preferred register for the LHS of an argument load to the argument register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(threeAddr->lhs) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->lhs] = threeAddr->imm;
				} else {
					preferredRegisters[threeAddr->lhs] = -1;
				}
				break;

			case IR::Entry::Type::StoreArg:
				// Set the preferred register for the RHS of an argument store to the argument register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(threeAddr->rhs1) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->rhs1] = threeAddr->imm;
				} else {
					preferredRegisters[threeAddr->rhs1] = -1;
				}
				break;
		}
	}

	return preferredRegisters;
}

/*!
 * \brief Spill a variable in a procedure to the stack
 * \param procedure Procedure to modify
 * \param symbol Symbol to spill
 * \param liveVariables Live variables in procedure
 * \param useDefs Use-def chains in procedure
 */
void spillVariable(IR::Procedure &procedure, IR::Symbol *symbol, Analysis::LiveVariables &liveVariables, Analysis::Analysis &analysis)
{
	int idx = 0;
	bool live = false;
	std::set<IR::Symbol*> liveSet;
	std::set<const IR::Entry*> neededDefs;
	std::set<IR::Entry*> spillLoads;

	Analysis::UseDefs &useDefs = analysis.useDefs();
	Analysis::Constants &constants = analysis.constants();

	// Iterate through the entries in the procedure
	for(IR::Entry *entry : procedure.entries()) {
		// If the entry uses the symbol and the symbol was not already live, the symbol must be
		// loaded from the stack
		if(entry->uses(symbol) && !live) {
			bool isConstant;
			int value = constants.getIntValue(entry, symbol, isConstant);

			IR::Entry *def;
			if(isConstant) {
				// If the entry was constant, then just rematerialize the constant
				def = new IR::EntryThreeAddr(IR::Entry::Type::Move, symbol, 0, 0, value);
			} else {
				// Otherwise, load it from its stack location
				def = new IR::EntryThreeAddr(IR::Entry::Type::LoadStack, symbol, 0, 0, idx);
				const std::set<const IR::Entry*> &defs = useDefs.defines(entry, symbol);
				neededDefs.insert(defs.begin(), defs.end());
			}

			// Insert the new instruction
			procedure.entries().insert(entry, def);
			spillLoads.insert(def);

			// The variable is now live
			live = true;
			liveSet = liveVariables.variables(entry);
		}

		// An assignment to the variable also makes it live
		if(entry->assign() == symbol) {
			live = true;
			liveSet = liveVariables.variables(entry);
		}

		if(entry->type == IR::Entry::Type::Label) {
			// Liveness ceases when the current block ends
			live = false;
		} else if(live) {
			// Liveness of the symbol must also cease if any variable goes dead.  If it did not
			// cease at that point, then spilling the variable would not be effective in reducing
			// register pressure
			std::set<IR::Symbol*> &currentVariables = liveVariables.variables(entry);
			for(IR::Symbol *s : liveSet) {
				if(currentVariables.find(s) == currentVariables.end()) {
					live = false;
					break;
				}
			}
			liveSet = currentVariables;
		}
	}

	// Now iterate through the procedure again
	for(IR::EntryList::iterator entryIt = procedure.entries().begin(); entryIt != procedure.entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;

		if(entry->assign() == symbol) {
			// If this entry assigns to the symbol, check whether the definition is still necessary
			if(neededDefs.find(entry) != neededDefs.end()) {
				// The definition is used.  It therefore has to be saved to the stack
				entryIt++;
				procedure.entries().insert(entryIt, new IR::EntryThreeAddr(IR::Entry::Type::StoreStack, 0, symbol, 0, idx));
				entryIt--;
			} else if(spillLoads.find(entry) == spillLoads.end()) {
				// All uses of this definition were rematerialized, so the definition is no
				// longer necessary at all
				entryIt--;
				procedure.entries().erase(entry);
			}
		}
	}

	// If a spill to the stack was necessary, increase the size of the stack frame in the
	// prologue and epilogue to the function
	if(neededDefs.size() > 0) {
		for(IR::Entry *entry : procedure.entries()) {
			if(entry->type == IR::Entry::Type::Prologue || entry->type == IR::Entry::Type::Epilogue) {
				IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
				idx = threeAddr->imm;
				threeAddr->imm++;
			}
		}
	}
}

/*!
 * \brief Allocate registers for a procedure
 * \param procedure Procedure to analyze
 * \return Map from symbol to register number for each symbol in the procedure
 */
std::map<IR::Symbol*, int> RegisterAllocator::allocate(IR::Procedure &procedure)
{
	std::map<IR::Symbol*, int> registers;
	bool success;

	Analysis::Analysis analysis(procedure);

	do {
		Transform::LiveRangeRenaming::instance()->transform(procedure, analysis);

		// Attempt an allocation
		registers = tryAllocate(procedure, success, analysis);

		if(!success) {
			Util::log("ir") << "*** IR (after spilling) ***" << std::endl;
			procedure.print(Util::log("ir"));
			Util::log("ir") << std::endl;
		}
	} while(!success);

	return registers;
}

/*!
 * \brief Attempt to allocate registers for a procedure
 * \param procedure Procedure to analyze
 * \param success [out] True if allocation was successful
 * \return Map from symbol to register number
 */
std::map<IR::Symbol*, int> RegisterAllocator::tryAllocate(IR::Procedure &procedure, bool &success, Analysis::Analysis &analysis)
{
	std::map<IR::Symbol*, int> registers;

	// Construct an interference graph, live variable list, and use-def chains for the procedure
	Analysis::LiveVariables liveVariables(procedure, analysis.flowGraph());
	Analysis::InterferenceGraph graph(procedure, &liveVariables);
	Analysis::UseDefs &useDefs = analysis.useDefs();

	// Construct artificial graph nodes for each register which is not preserved across procedure calls
	std::vector<IR::Symbol*> callerSavedRegisters;
	for(int i=0; i<CallerSavedRegisters; i++) {
		std::stringstream s;
		s << "arg" << i;
		std::unique_ptr<IR::Symbol> symbol = std::make_unique<IR::Symbol>(s.str(), 4, nullptr);
		registers[symbol.get()] = i;
		callerSavedRegisters.push_back(symbol.get());
		procedure.addSymbol(std::move(symbol));
	}

	// Add graph edges for all variables live across procedure calls
	addProcedureCallInterferences(graph, callerSavedRegisters, procedure, liveVariables);

	// Estimate spill costs for each symbol in the procedure
	std::map<IR::Symbol*, int> spillCosts = getSpillCosts(procedure, analysis.flowGraph());

	// Build a copy of the graph, so that it can be simplified
	std::vector<IR::Symbol*> stack;
	Analysis::InterferenceGraph simplifiedGraph(graph);
	bool spilled = false;

	// Operate on the graph until all nodes have been removed
	while(simplifiedGraph.symbols().size() > 0) {
		bool removed = false;
		IR::Symbol *spillCandidate = 0;

		// Iterate through the list of symbols in the procedure
		for(IR::Symbol *symbol : simplifiedGraph.symbols()) {
			// If the symbol has a lower spill cost than the previous spill candidate, save
			// it off as the new spill candidate
			if(!spillCandidate || spillCosts[symbol] < spillCosts[spillCandidate]) {
				spillCandidate = symbol;
			}

			// Check the set of interferences for this symbol
			const std::set<IR::Symbol*> &set = simplifiedGraph.interferences(symbol);
			if(set.size() < MaxRegisters) {
				// If the symbol has less than MaxRegister interferences, then it can be safely
				// removed from the graph, since there will always be a register available to
				// assign to it
				simplifiedGraph.removeSymbol(symbol);
				stack.push_back(symbol);
				removed = true;
				break;
			}
		}

		if(!removed) {
			// If no variable could be removed from the graph, then one needs to be spilled.
			// Spill the variable with the lowest spill cost that was determined above
			spillVariable(procedure, spillCandidate, liveVariables, analysis);
			simplifiedGraph.removeSymbol(spillCandidate);
			stack.push_back(spillCandidate);
			spilled = true;
		}
	}

	if(spilled) {
		// If a variable was spilled, the entire allocation procedure must be attempted again
		success = false;
		return registers;
	}

	// Determine which variables have a preferred register
	std::map<IR::Symbol*, int> preferredRegisters = getPreferredRegisters(procedure);

	// Reconstruct the graph by playing the stack in reverse
	while(!stack.empty()) {
		IR::Symbol *symbol = stack.back();
		stack.pop_back();

		// Find the set of interfering symbols for this one
		const std::set<IR::Symbol*> &set = graph.interferences(symbol);

		// Find a register which is not used by any of the interfering symbols.  This is
		// guaranteed to be possible by the way that the stack was constructed above.
		for(int i=-1; i<MaxRegisters; i++) {
			bool found = false;

			int reg = i;
			if(reg == -1) {
				// The first time through the loop, check for the availability of the symbol's
				// preferred register
				if(preferredRegisters.find(symbol) != preferredRegisters.end() && preferredRegisters[symbol] != -1) {
					reg = preferredRegisters[symbol];
				} else {
					continue;
				}
			}

			// Loop through the interfering symbols, and see if any of them use the current register number
			for(IR::Symbol *otherSymbol : set) {
				auto regIt = registers.find(otherSymbol);
				if(regIt != registers.end() && regIt->second == reg) {
					found = true;
					break;
				}
			}

			if(!found) {
				// An available register was found.  Assign the symbol to it.
				registers[symbol] = reg;
				break;
			}
		}
	}

	success = true;
	return registers;
}

}