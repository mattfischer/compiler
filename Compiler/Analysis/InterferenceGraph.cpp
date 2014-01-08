#include "Analysis/InterferenceGraph.h"

#include "Analysis/LiveVariables.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

namespace Analysis {

InterferenceGraph::SymbolSet emptySymbolSet;

InterferenceGraph::InterferenceGraph(IR::Procedure *procedure)
{
	LiveVariables liveVariables(procedure);

	liveVariables.print();

	for(IR::EntryList::iterator entryIt = procedure->entries().begin(); entryIt != procedure->entries().end(); entryIt++) {
		IR::Entry *entry = *entryIt;

		LiveVariables::SymbolSet &symbols = liveVariables.variables(entry);

		for(LiveVariables::SymbolSet::iterator symbolIt1 = symbols.begin(); symbolIt1 != symbols.end(); symbolIt1++) {
			IR::Symbol *symbol1 = *symbolIt1;

			for(LiveVariables::SymbolSet::iterator symbolIt2 = symbolIt1; symbolIt2 != symbols.end(); symbolIt2++) {
				IR::Symbol *symbol2 = *symbolIt2;

				addEdge(symbol1, symbol2);
			}
		}
	}
}

void InterferenceGraph::addEdge(IR::Symbol *symbol1, IR::Symbol *symbol2)
{
	if(symbol1 != symbol2) {
		mGraph[symbol1].insert(symbol2);
		mGraph[symbol2].insert(symbol1);
	}
}

void InterferenceGraph::removeSymbol(IR::Symbol *symbol)
{
	mGraph.erase(mGraph.find(symbol));

	for(SymbolToSymbolSetMap::iterator it = mGraph.begin(); it != mGraph.end(); it++) {
		SymbolSet &set = it->second;
		set.erase(set.find(symbol));
	}
}

const std::set<IR::Symbol*> &InterferenceGraph::interferences(IR::Symbol *symbol)
{
	SymbolToSymbolSetMap::const_iterator it = mGraph.find(symbol);

	if(it != mGraph.end()) {
		return it->second;
	} else {
		return emptySymbolSet;
	}
}

}