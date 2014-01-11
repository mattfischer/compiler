#include "Back/RegisterAllocator.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/InterferenceGraph.h"
#include "Analysis/LiveVariables.h"
#include "Analysis/Loops.h"
#include "Analysis/UseDefs.h"

#include "Front/Type.h"

#include <vector>
#include <sstream>

namespace Back {

static const int MaxRegisters = 13;
static const int CallerSavedRegisters = 4;

std::map<IR::Symbol*, int> getSpillCosts(IR::Procedure *procedure)
{
	std::map<IR::Symbol*, int> costs;

	Analysis::Loops loops(procedure);
	Analysis::Loops::Loop *rootLoop = loops.rootLoop();

	for(Analysis::FlowGraph::BlockSet::iterator blockIt = rootLoop->blocks.begin(); blockIt != rootLoop->blocks.end(); blockIt++) {
		Analysis::FlowGraph::Block *block = *blockIt;

		int cost = 1;
		for(Analysis::Loops::LoopList::iterator loopIt = loops.loops().begin(); loopIt != loops.loops().end(); loopIt++) {
			Analysis::Loops::Loop *loop = *loopIt;

			if(loop->blocks.find(block) != loop->blocks.end()) {
				cost *= 10;
			}
		}

		for(IR::EntrySubList::iterator entryIt = block->entries.begin(); entryIt != block->entries.end(); entryIt++) {
			IR::Entry *entry = *entryIt;

			for(IR::Procedure::SymbolList::iterator symbolIt = procedure->symbols().begin(); symbolIt != procedure->symbols().end(); symbolIt++) {
				IR::Symbol *symbol = *symbolIt;

				if(entry->assign() == symbol) {
					costs[symbol] += cost;
				}

				if(entry->uses(symbol)) {
					costs[symbol] += cost;
				}
			}
		}
	}

	return costs;
}

void addInterferences(Analysis::InterferenceGraph &graph, const Analysis::InterferenceGraph::SymbolSet &symbols, IR::Symbol *symbol, IR::Symbol *exclude)
{
	for(Analysis::LiveVariables::SymbolSet::const_iterator symbolIt = symbols.begin(); symbolIt != symbols.end(); symbolIt++) {
		IR::Symbol *otherSymbol = *symbolIt;
		if(otherSymbol != exclude) {
			graph.addEdge(symbol, otherSymbol);
		}
	}
}

void addProcedureCallInterferences(Analysis::InterferenceGraph &graph, const std::vector<IR::Symbol*> callerSavedRegisters, IR::Procedure *procedure, Analysis::LiveVariables &liveVariables)
{
	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;
		IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
		IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;

		const Analysis::LiveVariables::SymbolSet &variables = liveVariables.variables(entry);

		switch(entry->type) {
			case IR::Entry::TypeCall:
				for(unsigned int i=0; i<callerSavedRegisters.size(); i++) {
					addInterferences(graph, variables, callerSavedRegisters[i], 0);
				}
				break;

			case IR::Entry::TypeLoadRet:
				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->lhs);
				break;

			case IR::Entry::TypeStoreRet:
				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->rhs1);
				break;

			case IR::Entry::TypeLoadArg:
				addInterferences(graph, variables, callerSavedRegisters[twoAddrImm->imm], twoAddrImm->lhs);
				break;

			case IR::Entry::TypeStoreArg:
				addInterferences(graph, variables, callerSavedRegisters[twoAddrImm->imm], twoAddrImm->rhs);
				break;
		}
	}
}

std::map<IR::Symbol*, int> getPreferredRegisters(IR::Procedure *procedure)
{
	std::map<IR::Symbol*, int> preferredRegisters;

	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;
		IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
		IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;

		switch(entry->type) {
			case IR::Entry::TypeLoadRet:
				if(preferredRegisters.find(threeAddr->lhs) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->lhs] = 0;
				} else {
					preferredRegisters[threeAddr->lhs] = -1;
				}
				break;

			case IR::Entry::TypeStoreRet:
				if(preferredRegisters.find(threeAddr->rhs1) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->rhs1] = 0;
				} else {
					preferredRegisters[threeAddr->rhs1] = -1;
				}
				break;

			case IR::Entry::TypeLoadArg:
				if(preferredRegisters.find(twoAddrImm->lhs) == preferredRegisters.end()) {
					preferredRegisters[twoAddrImm->lhs] = twoAddrImm->imm;
				} else {
					preferredRegisters[twoAddrImm->lhs] = -1;
				}
				break;

			case IR::Entry::TypeStoreArg:
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

void spillVariable(IR::Procedure *procedure, IR::Symbol *symbol, Analysis::LiveVariables &liveVariables, Analysis::UseDefs &useDefs)
{
	int idx = 0;

	bool live = false;
	Analysis::LiveVariables::SymbolSet liveSet;
	IR::EntrySet neededDefs;
	IR::EntrySet spillLoads;

	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;

		if(entry->uses(symbol) && !live) {
			const IR::EntrySet &defs = useDefs.defines(entry, symbol);
			int value = 0;
			bool isConstant = false;
			for(IR::EntrySet::const_iterator defIt = defs.begin(); defIt != defs.end(); defIt++) {
				IR::Entry *def = *defIt;
				if(def->type == IR::Entry::TypeLoadImm) {
					IR::EntryOneAddrImm *oneAddrImm = (IR::EntryOneAddrImm*)def;
					if(!isConstant) {
						value = oneAddrImm->imm;
						isConstant = true;
					} else if(oneAddrImm->imm != value) {
						isConstant = false;
						break;
					}
				} else {
					isConstant = false;
					break;
				}
			}

			IR::Entry *def;
			if(isConstant) {
				def = new IR::EntryOneAddrImm(IR::Entry::TypeLoadImm, symbol, value);
			} else {
				def = new IR::EntryTwoAddrImm(IR::Entry::TypeLoadStack, symbol, 0, idx);
				neededDefs.insert(defs.begin(), defs.end());
			}
			procedure->entries().insert(entryIt, def);
			spillLoads.insert(def);
			live = true;
			liveSet = liveVariables.variables(entry);
		}

		if(entry->assign() == symbol) {
			live = true;
			liveSet = liveVariables.variables(entry);
		}

		if(entry->type == IR::Entry::TypeLabel) {
			live = false;
		} else if(live) {
			Analysis::LiveVariables::SymbolSet &currentVariables = liveVariables.variables(entry);
			for(Analysis::LiveVariables::SymbolSet::iterator symbolIt = liveSet.begin(); symbolIt != liveSet.end(); symbolIt++) {
				IR::Symbol *s = *symbolIt;

				if(currentVariables.find(s) == currentVariables.end()) {
					live = false;
					break;
				}
			}
			liveSet = currentVariables;
		}
	}

	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;

		if(entry->assign() == symbol) {
			if(neededDefs.find(entry) != neededDefs.end()) {
				entryIt++;
				procedure->entries().insert(entryIt, new IR::EntryTwoAddrImm(IR::Entry::TypeStoreStack, 0, symbol, idx));
				entryIt--;
			} else if(spillLoads.find(entry) == spillLoads.end()) {
				entryIt--;
				procedure->entries().erase(entry);
			}
		}
	}

	if(neededDefs.size() > 0) {
		for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
			IR::Entry *entry = *entryIt;

			if(entry->type == IR::Entry::TypePrologue || entry->type == IR::Entry::TypeEpilogue) {
				IR::EntryOneAddrImm *oneAddrImm = (IR::EntryOneAddrImm*)entry;
				idx = oneAddrImm->imm;
				oneAddrImm->imm++;
			}
		}
	}
}

std::map<IR::Symbol*, int> RegisterAllocator::allocate(IR::Procedure *procedure)
{
	std::map<IR::Symbol*, int> registers;
	bool success;

	do {
		registers = tryAllocate(procedure, success);
	} while(!success);

	return registers;
}

std::map<IR::Symbol*, int> RegisterAllocator::tryAllocate(IR::Procedure *procedure, bool &success)
{
	std::map<IR::Symbol*, int> registers;

	Analysis::InterferenceGraph graph(procedure);
	Analysis::LiveVariables liveVariables(procedure);
	Analysis::UseDefs useDefs(procedure);

	std::vector<IR::Symbol*> callerSavedRegisters;
	for(int i=0; i<CallerSavedRegisters; i++) {
		std::stringstream s;
		s << "arg" << i;
		IR::Symbol *symbol = new IR::Symbol(s.str(), Front::Type::find("int"));
		registers[symbol] = i;
		callerSavedRegisters.push_back(symbol);
	}

	addProcedureCallInterferences(graph, callerSavedRegisters, procedure, liveVariables);

	std::map<IR::Symbol*, int> spillCosts = getSpillCosts(procedure);

	std::vector<IR::Symbol*> stack;
	Analysis::InterferenceGraph simplifiedGraph(graph);
	bool spilled = false;

	while(simplifiedGraph.symbols().size() > 0) {
		bool removed = false;
		IR::Symbol *spillCandidate = 0;
		for(Analysis::InterferenceGraph::SymbolSet::const_iterator symbolIt = simplifiedGraph.symbols().begin(); symbolIt != simplifiedGraph.symbols().end(); symbolIt++) {
			IR::Symbol *symbol = *symbolIt;

			if(!spillCandidate || spillCosts[symbol] < spillCosts[spillCandidate]) {
				spillCandidate = symbol;
			}

			const Analysis::InterferenceGraph::SymbolSet &set = simplifiedGraph.interferences(symbol);
			if(set.size() < MaxRegisters) {
				simplifiedGraph.removeSymbol(symbol);
				stack.push_back(symbol);
				removed = true;
				break;
			}
		}

		if(!removed) {
			spillVariable(procedure, spillCandidate, liveVariables, useDefs);
			simplifiedGraph.removeSymbol(spillCandidate);
			stack.push_back(spillCandidate);
			spilled = true;
		}
	}

	if(spilled) {
		success = false;
		return registers;
	}

	std::map<IR::Symbol*, int> preferredRegisters = getPreferredRegisters(procedure);

	while(!stack.empty()) {
		IR::Symbol *symbol = stack.back();
		stack.pop_back();

		const Analysis::InterferenceGraph::SymbolSet &set = graph.interferences(symbol);

		for(int i=-1; i<MaxRegisters; i++) {
			bool found = false;

			int reg = i;
			if(reg == -1) {
				if(preferredRegisters.find(symbol) != preferredRegisters.end() && preferredRegisters[symbol] != -1) {
					reg = preferredRegisters[symbol];
				} else {
					continue;
				}
			}

			for(Analysis::InterferenceGraph::SymbolSet::const_iterator symbolIt = set.begin(); symbolIt != set.end(); symbolIt++) {
				IR::Symbol *otherSymbol = *symbolIt;
				std::map<IR::Symbol*, int>::iterator regIt = registers.find(otherSymbol);
				if(regIt != registers.end() && regIt->second == reg) {
					found = true;
					break;
				}
			}

			if(!found) {
				registers[symbol] = reg;
				break;
			}
		}
	}

	success = true;
	return registers;
}

}