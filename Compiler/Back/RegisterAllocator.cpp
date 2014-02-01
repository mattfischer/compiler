#include "Back/RegisterAllocator.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/InterferenceGraph.h"
#include "Analysis/LiveVariables.h"
#include "Analysis/Loops.h"
#include "Analysis/UseDefs.h"
#include "Analysis/Constants.h"

#include "Front/Type.h"

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
std::map<IR::Symbol*, int> getSpillCosts(IR::Procedure *procedure, Analysis::FlowGraph *flowGraph)
{
	std::map<IR::Symbol*, int> costs;

	// Perform flow graph and loop analysis on the procedure
	Analysis::Loops loops(procedure, flowGraph);
	Analysis::Loops::Loop *rootLoop = loops.rootLoop();

	// Iterate through the blocks of the procedure
	for(Analysis::FlowGraph::BlockSet::iterator blockIt = rootLoop->blocks.begin(); blockIt != rootLoop->blocks.end(); blockIt++) {
		Analysis::FlowGraph::Block *block = *blockIt;

		// Determine the cost of a variable use in this block, based on its loop depth.
		int cost = 1;
		for(Analysis::Loops::LoopList::iterator loopIt = loops.loops().begin(); loopIt != loops.loops().end(); loopIt++) {
			Analysis::Loops::Loop *loop = *loopIt;

			if(loop->blocks.find(block) != loop->blocks.end()) {
				cost *= 10;
			}
		}

		// Iterate through the block's entries
		for(IR::EntrySubList::iterator entryIt = block->entries.begin(); entryIt != block->entries.end(); entryIt++) {
			IR::Entry *entry = *entryIt;

			// Iterate through the symbols in the procedure
			for(IR::SymbolList::iterator symbolIt = procedure->symbols().begin(); symbolIt != procedure->symbols().end(); symbolIt++) {
				IR::Symbol *symbol = *symbolIt;

				// Any assignment to the variable adds to its cost
				if(entry->assign() == symbol) {
					costs[symbol] += cost;
				}

				// Any use of the variable also adds to its cost
				if(entry->uses(symbol)) {
					costs[symbol] += cost;
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
void addInterferences(Analysis::InterferenceGraph &graph, const IR::SymbolSet &symbols, IR::Symbol *symbol, IR::Symbol *exclude)
{
	for(IR::SymbolSet::const_iterator symbolIt = symbols.begin(); symbolIt != symbols.end(); symbolIt++) {
		IR::Symbol *otherSymbol = *symbolIt;
		if(otherSymbol != exclude) {
			graph.addEdge(symbol, otherSymbol);
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
void addProcedureCallInterferences(Analysis::InterferenceGraph &graph, const std::vector<IR::Symbol*> callerSavedRegisters, IR::Procedure *procedure, Analysis::LiveVariables &liveVariables)
{
	// Iterate through the procedure's entries
	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;
		IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
		IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;

		// Determine variables which are live in this entry
		const IR::SymbolSet &variables = liveVariables.variables(entry);

		switch(entry->type) {
			case IR::Entry::TypeCall:
				// A call creates interferences between all caller-saved registers and
				// all live variables
				for(unsigned int i=0; i<callerSavedRegisters.size(); i++) {
					addInterferences(graph, variables, callerSavedRegisters[i], 0);
				}
				break;

			case IR::Entry::TypeLoadRet:
				// A return load creates interferences with the return register for all variables
				// except the load's LHS
				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->lhs);
				break;

			case IR::Entry::TypeStoreRet:
				// A return store creates interferences with the return register for all variables
				// except the store's RHS
				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->rhs1);
				break;

			case IR::Entry::TypeLoadArg:
				// An argument load creates interferences with the argument register for all variables
				// except the load's LHS
				addInterferences(graph, variables, callerSavedRegisters[twoAddrImm->imm], twoAddrImm->lhs);
				break;

			case IR::Entry::TypeStoreArg:
				// An argument store creates interferences with the argument register for all variables
				// except the load's RHS
				addInterferences(graph, variables, callerSavedRegisters[twoAddrImm->imm], twoAddrImm->rhs);
				break;
		}
	}
}

/*!
 * \brief Get preferred registers for each symbol in a procedure
 * \param procedure Procedure to analyze
 * \return Map from symbol to preferred register number
 */
std::map<IR::Symbol*, int> getPreferredRegisters(IR::Procedure *procedure)
{
	std::map<IR::Symbol*, int> preferredRegisters;

	// Iterate through the procedure's entries
	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;
		IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
		IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;

		switch(entry->type) {
			case IR::Entry::TypeLoadRet:
				// Set the preferred register for the LHS of a return load to the return register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(threeAddr->lhs) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->lhs] = 0;
				} else {
					preferredRegisters[threeAddr->lhs] = -1;
				}
				break;

			case IR::Entry::TypeStoreRet:
				// Set the preferred register for the RHS of a return store to the return register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(threeAddr->rhs1) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->rhs1] = 0;
				} else {
					preferredRegisters[threeAddr->rhs1] = -1;
				}
				break;

			case IR::Entry::TypeLoadArg:
				// Set the preferred register for the LHS of an argument load to the argument register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(twoAddrImm->lhs) == preferredRegisters.end()) {
					preferredRegisters[twoAddrImm->lhs] = twoAddrImm->imm;
				} else {
					preferredRegisters[twoAddrImm->lhs] = -1;
				}
				break;

			case IR::Entry::TypeStoreArg:
				// Set the preferred register for the RHS of an argument store to the argument register,
				// or invalidate it if it had already been set to a different value
				if(preferredRegisters.find(twoAddrImm->rhs) == preferredRegisters.end()) {
					preferredRegisters[twoAddrImm->rhs] = twoAddrImm->imm;
				} else {
					preferredRegisters[twoAddrImm->rhs] = -1;
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
void spillVariable(IR::Procedure *procedure, IR::Symbol *symbol, Analysis::LiveVariables &liveVariables, Analysis::Analysis &analysis)
{
	int idx = 0;
	bool live = false;
	IR::SymbolSet liveSet;
	IR::EntrySet neededDefs;
	IR::EntrySet spillLoads;

	Analysis::UseDefs *useDefs = analysis.useDefs();
	Analysis::Constants *constants = analysis.constants();

	// Iterate through the entries in the procedure
	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;

		// If the entry uses the symbol and the symbol was not already live, the symbol must be
		// loaded from the stack
		if(entry->uses(symbol) && !live) {
			bool isConstant;
			int value = constants->getValue(entry, symbol, isConstant);

			IR::Entry *def;
			if(isConstant) {
				// If the entry was constant, then just rematerialize the constant
				def = new IR::EntryTwoAddrImm(IR::Entry::TypeLoadImm, symbol, 0, value);
			} else {
				// Otherwise, load it from its stack location
				def = new IR::EntryTwoAddrImm(IR::Entry::TypeLoadStack, symbol, 0, idx);
				const IR::EntrySet &defs = useDefs->defines(entry, symbol);
				neededDefs.insert(defs.begin(), defs.end());
			}

			// Insert the new instruction
			procedure->entries().insert(entryIt, def);
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

		if(entry->type == IR::Entry::TypeLabel) {
			// Liveness ceases when the current block ends
			live = false;
		} else if(live) {
			// Liveness of the symbol must also cease if any variable goes dead.  If it did not
			// cease at that point, then spilling the variable would not be effective in reducing
			// register pressure
			IR::SymbolSet &currentVariables = liveVariables.variables(entry);
			for(IR::SymbolSet::iterator symbolIt = liveSet.begin(); symbolIt != liveSet.end(); symbolIt++) {
				IR::Symbol *s = *symbolIt;

				if(currentVariables.find(s) == currentVariables.end()) {
					live = false;
					break;
				}
			}
			liveSet = currentVariables;
		}
	}

	// Now iterate through the procedure again
	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;

		if(entry->assign() == symbol) {
			// If this entry assigns to the symbol, check whether the definition is still necessary
			if(neededDefs.find(entry) != neededDefs.end()) {
				// The definition is used.  It therefore has to be saved to the stack
				entryIt++;
				procedure->entries().insert(entryIt, new IR::EntryTwoAddrImm(IR::Entry::TypeStoreStack, 0, symbol, idx));
				entryIt--;
			} else if(spillLoads.find(entry) == spillLoads.end()) {
				// All uses of this definition were rematerialized, so the definition is no
				// longer necessary at all
				entryIt--;
				procedure->entries().erase(entry);
			}
		}
	}

	// If a spill to the stack was necessary, increase the size of the stack frame in the
	// prologue and epilogue to the function
	if(neededDefs.size() > 0) {
		for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
			IR::Entry *entry = *entryIt;

			if(entry->type == IR::Entry::TypePrologue || entry->type == IR::Entry::TypeEpilogue) {
				IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;
				idx = twoAddrImm->imm;
				twoAddrImm->imm++;
			}
		}
	}
}

/*!
 * \brief Allocate registers for a procedure
 * \param procedure Procedure to analyze
 * \return Map from symbol to register number for each symbol in the procedure
 */
std::map<IR::Symbol*, int> RegisterAllocator::allocate(IR::Procedure *procedure)
{
	std::map<IR::Symbol*, int> registers;
	bool success;

	Analysis::Analysis analysis(procedure);

	do {
		// Attempt an allocation
		registers = tryAllocate(procedure, success, analysis);

		if(!success) {
			std::cout << "*** IR (after spilling) ***" << std::endl;
			procedure->print();
			std::cout << std::endl;
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
std::map<IR::Symbol*, int> RegisterAllocator::tryAllocate(IR::Procedure *procedure, bool &success, Analysis::Analysis &analysis)
{
	std::map<IR::Symbol*, int> registers;

	// Construct an interference graph, live variable list, and use-def chains for the procedure
	Analysis::LiveVariables liveVariables(procedure, analysis.flowGraph());
	Analysis::InterferenceGraph graph(procedure, &liveVariables);
	Analysis::UseDefs *useDefs = analysis.useDefs();

	// Construct artificial graph nodes for each register which is not preserved across procedure calls
	std::vector<IR::Symbol*> callerSavedRegisters;
	for(int i=0; i<CallerSavedRegisters; i++) {
		std::stringstream s;
		s << "arg" << i;
		IR::Symbol *symbol = new IR::Symbol(s.str(), Front::Type::find("int"));
		registers[symbol] = i;
		callerSavedRegisters.push_back(symbol);
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
		for(IR::SymbolSet::const_iterator symbolIt = simplifiedGraph.symbols().begin(); symbolIt != simplifiedGraph.symbols().end(); symbolIt++) {
			IR::Symbol *symbol = *symbolIt;

			// If the symbol has a lower spill cost than the previous spill candidate, save
			// it off as the new spill candidate
			if(!spillCandidate || spillCosts[symbol] < spillCosts[spillCandidate]) {
				spillCandidate = symbol;
			}

			// Check the set of interferences for this symbol
			const IR::SymbolSet &set = simplifiedGraph.interferences(symbol);
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
		const IR::SymbolSet &set = graph.interferences(symbol);

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
			for(IR::SymbolSet::const_iterator symbolIt = set.begin(); symbolIt != set.end(); symbolIt++) {
				IR::Symbol *otherSymbol = *symbolIt;
				std::map<IR::Symbol*, int>::iterator regIt = registers.find(otherSymbol);
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