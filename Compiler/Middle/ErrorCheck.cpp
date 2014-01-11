#include "Middle/ErrorCheck.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Analysis/LiveVariables.h"
#include "Analysis/FlowGraph.h"

#include <iostream>

namespace Middle {
	bool ErrorCheck::check(IR::Program *program)
	{
		for(IR::Program::ProcedureList::iterator it = program->procedures().begin(); it != program->procedures().end(); it++) {
			IR::Procedure *procedure = *it;

			Analysis::LiveVariables liveVariables(procedure);
			Analysis::LiveVariables::SymbolSet symbols = liveVariables.variables(procedure->entries().front());
			if(symbols.size() != 0) {
				IR::Symbol *symbol = *symbols.begin();
				std::cout << "Error: Use of undefined variable " << symbol->name << " in procedure " << procedure->name() << std::endl;
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

				bool foundRet = false;
				for(IR::EntrySubList::reverse_iterator entryIt = block->entries.rbegin(); entryIt != block->entries.rend(); entryIt++) {
					IR::Entry *entry = *entryIt;
					if(entry->type == IR::Entry::TypeStoreRet) {
						foundRet = true;
						break;
					}
				}

				if(foundRet) {
					continue;
				} else {
					blockList.insert(blockList.end(), block->pred.begin(), block->pred.end());
				}

				if(block == flowGraph.start()) {
					std::cout << "Error: Control reaches end of procedure " << procedure->name() << " without return" << std::endl;
					return false;
				}

				seenBlocks.insert(block);
			}
		}

		return true;
	}
}