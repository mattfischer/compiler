#include "Analysis/LiveVariables.h"

#include "Analysis/DataFlow.h"

#include "IR/Procedure.h"
#include "IR/Symbol.h"

namespace Analysis {
	LiveVariables::LiveVariables(IR::Procedure *procedure)
	{
		FlowGraph graph(procedure);
		mProcedure = procedure;

		IR::SymbolSet all(procedure->symbols().begin(), procedure->symbols().end());
		std::map<IR::Entry*, IR::SymbolSet> gen;
		std::map<IR::Entry*, IR::SymbolSet> kill;

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			for(IR::SymbolSet::iterator itSymbol = all.begin(); itSymbol != all.end(); itSymbol++) {
				IR::Symbol *symbol = *itSymbol;

				IR::SymbolSet &g = gen[entry];
				if(entry->uses(symbol)) {
					g.insert(symbol);
				}

				IR::Symbol *assign = entry->assign();
				if(assign && g.find(assign) == g.end()) {
					kill[entry].insert(assign);
				}
			}
		}

		DataFlow<IR::Symbol*> dataFlow;
		mMap = dataFlow.analyze(graph, gen, kill, all, DataFlow<IR::Symbol*>::MeetTypeUnion, DataFlow<IR::Symbol*>::DirectionBackward);
	}

	IR::SymbolSet &LiveVariables::variables(IR::Entry *entry)
	{
		return mMap[entry];
	}

	void LiveVariables::print()
	{
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			std::cout << *entry << " | ";
			IR::SymbolSet &symbols = variables(entry);
			for(IR::SymbolSet::iterator itSymbol = symbols.begin(); itSymbol != symbols.end(); itSymbol++) {
				IR::Symbol *symbol = *itSymbol;
				std::cout << symbol->name << " ";
			}
			std::cout << std::endl;
		}
	}
}