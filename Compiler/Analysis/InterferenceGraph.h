#ifndef ANALYSIS_INTERFERENCE_GRAPH_H
#define ANALYSIS_INTERFERENCE_GRAPH_H

#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Analysis/LiveVariables.h"

#include <map>

namespace Analysis {
/*!
 * \brief Graph of interferences between live variable ranges
 *
 * The interference graph is used to represent the sets of variables which
 * are ever live at the same time in a procedure.  An edge between two symbols
 * means they are simultaneously live at some point in the procedure, the absence
 * of an edge means they are not
 */
class InterferenceGraph {
public:
	InterferenceGraph(const IR::Procedure &procedure, LiveVariables *liveVariables);

	void addSymbol(IR::Symbol *symbol);
	void addEdge(IR::Symbol *symbol1, IR::Symbol *symbol2);
	void removeSymbol(IR::Symbol *symbol);

	const std::set<IR::Symbol*> &interferences(IR::Symbol *symbol);
	const std::set<IR::Symbol*> &symbols();

private:
	typedef std::map<IR::Symbol*, std::set<IR::Symbol*>> SymbolToSymbolSetMap;

	SymbolToSymbolSetMap mGraph; //!< Map of graph edges
	std::set<IR::Symbol*> mSymbols; //!< Set of symbols in the graph
};

}

#endif