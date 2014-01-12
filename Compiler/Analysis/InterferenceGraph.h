#ifndef ANALYSIS_INTERFERENCE_GRAPH_H
#define ANALYSIS_INTERFERENCE_GRAPH_H

#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include <map>

namespace Analysis {

class InterferenceGraph {
public:
	InterferenceGraph(IR::Procedure *procedure);

	void addSymbol(IR::Symbol *symbol);
	void addEdge(IR::Symbol *symbol1, IR::Symbol *symbol2);
	void removeSymbol(IR::Symbol *symbol);

	const IR::SymbolSet &interferences(IR::Symbol *symbol);
	const IR::SymbolSet &symbols();

private:
	typedef std::map<IR::Symbol*, IR::SymbolSet> SymbolToSymbolSetMap;

	SymbolToSymbolSetMap mGraph;
	IR::SymbolSet mSymbols;
};

}

#endif