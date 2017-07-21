#include "Analysis/LiveVariables.h"

#include "Analysis/DataFlow.h"

#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Util/Timer.h"
#include "Util/Log.h"

namespace Analysis {
	/*!
	 * \brief Constructor
	 * \param procedure Procedure to analyze
	 */
	LiveVariables::LiveVariables(IR::Procedure *procedure, FlowGraph &flowGraph)
		: mProcedure(procedure)
	{
		Util::Timer timer;
		timer.start();

		// Construct the set of all variables in the procedure
		IR::SymbolSet all(procedure->symbols().begin(), procedure->symbols().end());

		// Construct gen/kill sets for data flow analysis.
		std::map<IR::Entry*, IR::SymbolSet> gen;
		std::map<IR::Entry*, IR::SymbolSet> kill;
		for(IR::Entry *entry : procedure->entries()) {
			for(IR::Symbol *symbol : all) {
				IR::SymbolSet &g = gen[entry];
				if(entry->uses(symbol)) {
					// An entry which uses a symbol adds that symbol to the set of live symbols
					g.insert(symbol);
				}

				IR::Symbol *assign = entry->assign();
				if(assign && g.find(assign) == g.end()) {
					// An entry which assigns to a symbol and does not use it kills that symbol
					// from the live symbol set
					kill[entry].insert(assign);
				}
			}
		}

		// Perform a backwards data flow analysis on the procedure, using the gen and kill sets
		// constructed above.
		DataFlow<IR::Symbol*> dataFlow;
		mMap = dataFlow.analyze(flowGraph, gen, kill, all, DataFlow<IR::Symbol*>::Meet::Union, DataFlow<IR::Symbol*>::Direction::Backward);

		Util::log("opt.time") << "  LiveVariables(" << procedure->name() << "): " << timer.stop() << "ms" << std::endl;
	}

	/*!
	 * \brief Return the set of symbols live at any point
	 * \param entry Entry to return information for
	 * \return Set of live symbols
	 */
	IR::SymbolSet &LiveVariables::variables(IR::Entry *entry)
	{
		return mMap[entry];
	}

	/*!
	 * \brief Print out live variable information
	 */
	void LiveVariables::print(std::ostream &o)
	{
		// Loop through the procedure, printing out each entry along with the variables live at that point
		for(IR::Entry *entry : mProcedure->entries()) {
			o << *entry << " | ";
			IR::SymbolSet &symbols = variables(entry);
			for(IR::Symbol *symbol : symbols) {
				o << symbol->name << " ";
			}
			o << std::endl;
		}
	}
}