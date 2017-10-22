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
	LiveVariables::LiveVariables(const IR::Procedure &procedure, const FlowGraph &flowGraph)
		: mProcedure(procedure)
	{
		Util::Timer timer;
		timer.start();

		// Construct the set of all variables in the procedure
		std::set<const IR::Symbol*> all;
		for(const std::unique_ptr<IR::Symbol> &symbol : mProcedure.symbols()) {
			all.insert(symbol.get());
		}

		// Construct gen/kill sets for data flow analysis.
		std::map<const IR::Entry*, std::set<const IR::Symbol*>> gen;
		std::map<const IR::Entry*, std::set<const IR::Symbol*>> kill;
		for(const IR::Entry *entry : mProcedure.entries()) {
			for(const IR::Symbol *symbol : all) {
				std::set<const IR::Symbol*> &g = gen[entry];
				if(entry->uses(symbol)) {
					// An entry which uses a symbol adds that symbol to the set of live symbols
					g.insert(symbol);
				}

				const IR::Symbol *assign = entry->assign();
				if(assign && g.find(assign) == g.end()) {
					// An entry which assigns to a symbol and does not use it kills that symbol
					// from the live symbol set
					kill[entry].insert(assign);
				}
			}
		}

		// Perform a backwards data flow analysis on the procedure, using the gen and kill sets
		// constructed above.
		DataFlow<const IR::Symbol*> dataFlow;
		mMap = dataFlow.analyze(flowGraph, gen, kill, all, DataFlow<const IR::Symbol*>::Meet::Union, DataFlow<const IR::Symbol*>::Direction::Backward);

		Util::log("opt.time") << "  LiveVariables(" << mProcedure.name() << "): " << timer.stop() << "ms" << std::endl;
	}

	/*!
	 * \brief Return the set of symbols live at any point
	 * \param entry Entry to return information for
	 * \return Set of live symbols
	 */
	std::set<const IR::Symbol*> &LiveVariables::variables(const IR::Entry *entry)
	{
		return mMap[entry];
	}

	/*!
	 * \brief Print out live variable information
	 */
	void LiveVariables::print(std::ostream &o)
	{
		// Loop through the procedure, printing out each entry along with the variables live at that point
		for(const IR::Entry *entry : mProcedure.entries()) {
			o << *entry << " | ";
			std::set<const IR::Symbol*> &symbols = variables(entry);
			for(const IR::Symbol *symbol : symbols) {
				o << symbol->name << " ";
			}
			o << std::endl;
		}
	}
}