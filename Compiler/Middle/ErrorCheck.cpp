#include "Middle/ErrorCheck.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Analysis/LiveVariables.h"
#include "Analysis/FlowGraph.h"

namespace Middle {
	bool ErrorCheck::check(IR::Program *program)
	{
		for(IR::Program::ProcedureList::iterator it = program->procedures().begin(); it != program->procedures().end(); it++) {
			IR::Procedure *procedure = *it;

			Analysis::LiveVariables liveVariables(procedure);
			Analysis::LiveVariables::SymbolSet symbols = liveVariables.variables(procedure->entries().front());
			if(symbols.size() != 0) {
				IR::Symbol *symbol = *symbols.begin();
				printf("Error: Use of undefined variable %s in procedure %s\n", symbol->name.c_str(), procedure->name().c_str());
				return false;
			}

			Analysis::FlowGraph flowGraph(procedure);
			Analysis::FlowGraph::BlockList blockList;
			Analysis::FlowGraph::BlockSet seenBlocks;
			Analysis::FlowGraph::Block *end = flowGraph.end();
			blockList.insert(blockList.begin(), end->pred.begin(), end->pred.end());
			for(Analysis::FlowGraph::BlockList::iterator blockIt = blockList.begin(); blockIt != blockList.end(); blockIt++) {
				Analysis::FlowGraph::Block *block = *blockIt;

				if(seenBlocks.find(block) != seenBlocks.end()) {
					continue;
				}

				IR::Entry *endEntry = block->entries.back();
				if(endEntry->type == IR::Entry::TypeReturn) {
					continue;
				} else {
					blockList.insert(blockList.end(), block->pred.begin(), block->pred.end());
				}

				if(block == flowGraph.start()) {
					printf("Error: Control reaches end of procedure %s without return\n", procedure->name().c_str());
					return false;
				}

				seenBlocks.insert(block);
			}
		}

		return true;
	}
}