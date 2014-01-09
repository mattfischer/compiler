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

	for(IR::Procedure::SymbolList::iterator symbolIt = procedure->symbols().begin(); symbolIt != procedure->symbols().end(); symbolIt++) {
		IR::Symbol *symbol = *symbolIt;
		addSymbol(symbol);
	}

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

void InterferenceGraph::addSymbol(IR::Symbol *symbol)
{
	if(mGraph.find(symbol) == mGraph.end()) {
		mGraph[symbol] = emptySymbolSet;
		mSymbols.insert(symbol);
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
	mSymbols.erase(mSymbols.find(symbol));

	for(SymbolToSymbolSetMap::iterator it = mGraph.begin(); it != mGraph.end(); it++) {
		SymbolSet &set = it->second;
		if(set.find(symbol) != set.end()) {
			set.erase(set.find(symbol));
		}
	}
}

const InterferenceGraph::SymbolSet &InterferenceGraph::interferences(IR::Symbol *symbol)
{
	SymbolToSymbolSetMap::const_iterator it = mGraph.find(symbol);

	if(it != mGraph.end()) {
		return it->second;
	} else {
		return emptySymbolSet;
	}
}

const InterferenceGraph::SymbolSet &InterferenceGraph::symbols()
{
	return mSymbols;
}

}