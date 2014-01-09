#include "Back/RegisterAllocator.h"

#include "IR/Symbol.h"
#include "IR/Procedure.h"

#include "Analysis/InterferenceGraph.h"

#include <vector>

namespace Back {

static const int MaxRegisters = 13;

std::map<IR::Symbol*, int> RegisterAllocator::allocate(IR::Procedure *procedure)
{
	std::map<IR::Symbol*, int> registers;
	Analysis::InterferenceGraph graph(procedure);

	std::vector<IR::Symbol*> stack;
	Analysis::InterferenceGraph simplifiedGraph(graph);

	while(!simplifiedGraph.symbols().empty()) {
		for(Analysis::InterferenceGraph::SymbolSet::const_iterator symbolIt = simplifiedGraph.symbols().begin(); symbolIt != simplifiedGraph.symbols().end(); symbolIt++) {
			IR::Symbol *symbol = *symbolIt;

			const Analysis::InterferenceGraph::SymbolSet &set = simplifiedGraph.interferences(symbol);
			if(set.size() < MaxRegisters) {
				simplifiedGraph.removeSymbol(symbol);
				stack.push_back(symbol);
				break;
			}
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