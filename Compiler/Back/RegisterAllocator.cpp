#include "Back/RegisterAllocator.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/InterferenceGraph.h"
#include "Analysis/LiveVariables.h"

#include "Front/Type.h"

#include <vector>
#include <sstream>

namespace Back {

static const int MaxRegisters = 13;
static const int CallerSavedRegisters = 4;

void addInterferences(Analysis::InterferenceGraph &graph, const Analysis::InterferenceGraph::SymbolSet &symbols, IR::Symbol *symbol, IR::Symbol *exclude)
{
	for(Analysis::LiveVariables::SymbolSet::const_iterator symbolIt = symbols.begin(); symbolIt != symbols.end(); symbolIt++) {
		IR::Symbol *otherSymbol = *symbolIt;
		if(otherSymbol != exclude) {
			graph.addEdge(symbol, otherSymbol);
		}
	}
}

void spillVariable(IR::Procedure *procedure, IR::Symbol *symbol)
{
	int idx = 0;

	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;

		if(entry->type == IR::Entry::TypePrologue || entry->type == IR::Entry::TypeEpilogue) {
			IR::EntryOneAddrImm *oneAddrImm = (IR::EntryOneAddrImm*)entry;
			idx = oneAddrImm->imm;
			oneAddrImm->imm++;
		}

		if(entry->uses(symbol)) {
			procedure->entries().insert(entryIt, new IR::EntryTwoAddrImm(IR::Entry::TypeLoadStack, symbol, 0, idx));
		}

		if(entry->assign() == symbol) {
			entryIt++;
			procedure->entries().insert(entryIt, new IR::EntryTwoAddrImm(IR::Entry::TypeStoreStack, 0, symbol, idx));
			entryIt--;
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
	std::map<IR::Symbol*, int> preferredRegisters;

	Analysis::InterferenceGraph graph(procedure);
	Analysis::LiveVariables liveVariables(procedure);

	printf("Try Allocate:\n");
	procedure->print();
	printf("\n");

	std::vector<IR::Symbol*> callerSavedRegisters;
	for(int i=0; i<CallerSavedRegisters; i++) {
		std::stringstream s;
		s << "arg" << i;
		IR::Symbol *symbol = new IR::Symbol(s.str(), Front::Type::find("int"));
		registers[symbol] = i;
		procedure->addSymbol(symbol);
		callerSavedRegisters.push_back(symbol);
	}

	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;
		IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
		IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;

		const Analysis::LiveVariables::SymbolSet &variables = liveVariables.variables(entry);

		switch(entry->type) {
			case IR::Entry::TypeCall:
				for(int i=0; i<CallerSavedRegisters; i++) {
					addInterferences(graph, variables, callerSavedRegisters[i], 0);
				}
				break;

			case IR::Entry::TypeLoadRet:
				if(preferredRegisters.find(threeAddr->lhs) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->lhs] = 0;
				} else {
					preferredRegisters[threeAddr->lhs] = -1;
				}

				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->lhs);
				break;

			case IR::Entry::TypeStoreRet:
				if(preferredRegisters.find(threeAddr->rhs1) == preferredRegisters.end()) {
					preferredRegisters[threeAddr->rhs1] = 0;
				} else {
					preferredRegisters[threeAddr->rhs1] = -1;
				}

				addInterferences(graph, variables, callerSavedRegisters[0], threeAddr->rhs1);
				break;

			case IR::Entry::TypeLoadArg:
				if(preferredRegisters.find(twoAddrImm->lhs) == preferredRegisters.end()) {
					preferredRegisters[twoAddrImm->lhs] = twoAddrImm->imm;
				} else {
					preferredRegisters[twoAddrImm->lhs] = -1;
				}

				addInterferences(graph, variables, callerSavedRegisters[twoAddrImm->imm], twoAddrImm->lhs);
				break;

			case IR::Entry::TypeStoreArg:
				if(preferredRegisters.find(twoAddrImm->rhs) == preferredRegisters.end()) {
					preferredRegisters[twoAddrImm->rhs] = twoAddrImm->imm;
				} else {
					preferredRegisters[twoAddrImm->rhs] = -1;
				}

				addInterferences(graph, variables, callerSavedRegisters[twoAddrImm->imm], twoAddrImm->rhs);
				break;
		}
	}

	std::vector<IR::Symbol*> stack;
	Analysis::InterferenceGraph simplifiedGraph(graph);

	bool spilled = false;

	while(true) {
		bool removed = false;
		bool foundUnallocated = false;
		IR::Symbol *spillCandidate = 0;
		for(Analysis::InterferenceGraph::SymbolSet::const_iterator symbolIt = simplifiedGraph.symbols().begin(); symbolIt != simplifiedGraph.symbols().end(); symbolIt++) {
			IR::Symbol *symbol = *symbolIt;

			if(registers.find(symbol) == registers.end()) {
				foundUnallocated = true;
				if(!spillCandidate) {
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
		}

		if(foundUnallocated) {
			if(!removed) {
				spillVariable(procedure, spillCandidate);
				simplifiedGraph.removeSymbol(spillCandidate);
				stack.push_back(spillCandidate);
				spilled = true;
			}
		} else {
			break;
		}
	}

	if(spilled) {
		success = false;
		return registers;
	}

	while(!stack.empty()) {
		IR::Symbol *symbol = stack.back();
		stack.pop_back();

		const Analysis::InterferenceGraph::SymbolSet &set = graph.interferences(symbol);

		for(int i=-1; i<MaxRegisters; i++) {
			bool found = false;

			int reg;
			if(i == -1) {
				if(preferredRegisters.find(symbol) != preferredRegisters.end() && preferredRegisters[symbol] != -1) {
					reg = preferredRegisters[symbol];
				} else {
					continue;
				}
			} else {
				reg = i;
			}

			for(Analysis::InterferenceGraph::SymbolSet::const_iterator symbolIt = set.begin(); symbolIt != set.end(); symbolIt++) {
				IR::Symbol *otherSymbol = *symbolIt;
				if(registers.find(otherSymbol) != registers.end() && registers.find(otherSymbol)->second == reg) {
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