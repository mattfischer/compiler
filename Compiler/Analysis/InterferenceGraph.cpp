#include "Analysis/InterferenceGraph.h"

#include "Analysis/LiveVariables.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"

namespace Analysis {

std::set<const IR::Symbol*> emptySymbolSet; //!< Empty set, to use when a symbol lookup fails

/*!
 * \brief Constructor
 * \param procedure Procedure to analyze
 */
InterferenceGraph::InterferenceGraph(const IR::Procedure &procedure, LiveVariables *liveVariables)
{
	// Collect the set of all symbols in the procedure
	for(const std::unique_ptr<IR::Symbol> &symbol : procedure.symbols()) {
		addSymbol(symbol.get());
	}

	// Walk through the procedure.  For each entry, add graph edges between all variables live at that point
	for(const IR::Entry *entry : procedure.entries()) {
		std::set<const IR::Symbol*> &symbols = liveVariables->variables(entry);

		// Loop through the symbol set, adding edges
		for(auto symbolIt1 = symbols.begin(); symbolIt1 != symbols.end(); symbolIt1++) {
			const IR::Symbol *symbol1 = *symbolIt1;

			for(auto symbolIt2 = symbolIt1; symbolIt2 != symbols.end(); symbolIt2++) {
				const IR::Symbol *symbol2 = *symbolIt2;

				addEdge(symbol1, symbol2);
			}
		}
	}
}

/*!
 * \brief Add a symbol to the graph
 * \param symbol Symbol to add
 */
void InterferenceGraph::addSymbol(const IR::Symbol *symbol)
{
	if(mGraph.find(symbol) == mGraph.end()) {
		mGraph[symbol] = emptySymbolSet;
		mSymbols.insert(symbol);
	}
}

/*!
 * \brief Add an edge between two symbols in the graph
 * \param symbol1 First symbol
 * \param symbol2 Second symbol
 */
void InterferenceGraph::addEdge(const IR::Symbol *symbol1, const IR::Symbol *symbol2)
{
	if(symbol1 != symbol2) {
		// Add symbol1 -> symbol2
		auto it = mGraph.find(symbol1);
		if(it != mGraph.end()) {
			it->second.insert(symbol2);
		}

		// Add symbol2 -> symbol1
		it = mGraph.find(symbol2);
		if(it != mGraph.end()) {
			it->second.insert(symbol1);
		}
	}
}

/*!
 * \brief Remove a symbol from the graph
 * \param symbol Symbol to remove
 */
void InterferenceGraph::removeSymbol(const IR::Symbol *symbol)
{
	// Remove the symbol from the graph and symbol list
	mGraph.erase(mGraph.find(symbol));
	mSymbols.erase(mSymbols.find(symbol));

	// Loop through the rest of the symbol sets, removing the symbol from any which contain it
	for(auto &edge : mGraph) {
		std::set<const IR::Symbol*> &set = edge.second;
		if(set.find(symbol) != set.end()) {
			set.erase(set.find(symbol));
		}
	}
}

/*!
 * \brief Return the set of symbols which interfere with a given symbol
 * \param symbol Symbol to examine
 * \return Set of interfering symbols
 */
const std::set<const IR::Symbol*> &InterferenceGraph::interferences(const IR::Symbol *symbol)
{
	auto it = mGraph.find(symbol);

	if(it != mGraph.end()) {
		return it->second;
	} else {
		return emptySymbolSet;
	}
}

/*!
 * \brief Return the set of symbols in use in the graph
 * \return Symbols
 */
const std::set<const IR::Symbol*> &InterferenceGraph::symbols()
{
	return mSymbols;
}

}