#ifndef ANALYSIS_INTERFERENCE_GRAPH_H
#define ANALYSIS_INTERFERENCE_GRAPH_H

#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include <set>
#include <map>

namespace IR {
	class Procedure;
	class Symbol;
}

namespace Analysis {

class InterferenceGraph {
public:
	InterferenceGraph(IR::Procedure *procedure);

	void addSymbol(IR::Symbol *symbol);
	void addEdge(IR::Symbol *symbol1, IR::Symbol *symbol2);
	void removeSymbol(IR::Symbol *symbol);

	typedef std::set<IR::Symbol*> SymbolSet;
	const SymbolSet &interferences(IR::Symbol *symbol);
	const SymbolSet &symbols();

private:
	typedef std::map<IR::Symbol*, SymbolSet> SymbolToSymbolSetMap;

	SymbolToSymbolSetMap mGraph;
	SymbolSet mSymbols;
};

}

#endif