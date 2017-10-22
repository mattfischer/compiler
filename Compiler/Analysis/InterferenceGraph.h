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
	InterferenceGraph(const IR::Procedure &procedure, const LiveVariables &liveVariables);

	void addSymbol(const IR::Symbol *symbol);
	void addEdge(const IR::Symbol *symbol1, const IR::Symbol *symbol2);
	void removeSymbol(const IR::Symbol *symbol);

	const std::set<const IR::Symbol*> &interferences(const IR::Symbol *symbol);
	const std::set<const IR::Symbol*> &symbols();

private:
	std::map<const IR::Symbol*, std::set<const IR::Symbol*>> mGraph; //!< Map of graph edges
	std::set<const IR::Symbol*> mSymbols; //!< Set of symbols in the graph
};

}

#endif