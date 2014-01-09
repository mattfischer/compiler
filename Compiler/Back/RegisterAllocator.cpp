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

std::map<IR::Symbol*, int> RegisterAllocator::allocate(IR::Procedure *procedure)
{
	std::map<IR::Symbol*, int> registers;
	Analysis::InterferenceGraph graph(procedure);
	Analysis::LiveVariables liveVariables(procedure);

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
		const Analysis::LiveVariables::SymbolSet &variables = liveVariables.variables(entry);
		for(Analysis::LiveVariables::SymbolSet::const_iterator symbolIt = variables.begin(); symbolIt != variables.end(); symbolIt++) {
			IR::Symbol *symbol = *symbolIt;
			IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)entry;
			IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)entry;

			switch(entry->type) {
				case IR::Entry::TypeCall:
					for(int i=0; i<CallerSavedRegisters; i++) {
						graph.addEdge(symbol, callerSavedRegisters[i]);
					}
					break;

				case IR::Entry::TypeLoadRet:
					if(symbol != threeAddr->lhs) {
						graph.addEdge(symbol, callerSavedRegisters[0]);
					}
					break;

				case IR::Entry::TypeStoreRet:
					if(symbol != threeAddr->rhs1) {
						graph.addEdge(symbol, callerSavedRegisters[0]);
					}
					break;

				case IR::Entry::TypeLoadArg:
					if(symbol != twoAddrImm->lhs) {
						graph.addEdge(symbol, callerSavedRegisters[twoAddrImm->imm]);
					}
					break;

				case IR::Entry::TypeStoreArg:
					if(symbol != twoAddrImm->rhs) {
						graph.addEdge(symbol, callerSavedRegisters[twoAddrImm->imm]);
					}
					break;
			}
		}
	}

	std::vector<IR::Symbol*> stack;
	Analysis::InterferenceGraph simplifiedGraph(graph);

	while(true) {
		bool found = false;
		for(Analysis::InterferenceGraph::SymbolSet::const_iterator symbolIt = simplifiedGraph.symbols().begin(); symbolIt != simplifiedGraph.symbols().end(); symbolIt++) {
			IR::Symbol *symbol = *symbolIt;

			if(registers.find(symbol) == registers.end()) {
				const Analysis::InterferenceGraph::SymbolSet &set = simplifiedGraph.interferences(symbol);
				if(set.size() < MaxRegisters) {
					simplifiedGraph.removeSymbol(symbol);
					stack.push_back(symbol);
					found = true;
					break;
				}
			}
		}

		if(!found) {
			break;
		}
	}

	while(!stack.empty()) {
		IR::Symbol *symbol = stack.back();
		stack.pop_back();

		const Analysis::InterferenceGraph::SymbolSet &set = graph.interferences(symbol);

		for(int i=0; i<MaxRegisters; i++) {
			bool found = false;

			for(Analysis::InterferenceGraph::SymbolSet::const_iterator symbolIt = set.begin(); symbolIt != set.end(); symbolIt++) {
				IR::Symbol *otherSymbol = *symbolIt;
				if(registers.find(otherSymbol) != registers.end() && registers.find(otherSymbol)->second == i) {
					found = true;
					break;
				}
			}

			if(!found) {
				registers[symbol] = i;
				break;
			}
		}
	}

	return registers;
}

}