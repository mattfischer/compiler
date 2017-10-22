#include "Middle/ErrorCheck.h"

#include "Front/Symbol.h"

#include "IR/Program.h"
#include "IR/Procedure.h"
#include "IR/Symbol.h"

#include "Analysis/Analysis.h"
#include "Analysis/LiveVariables.h"

#include <iostream>
#include <sstream>

namespace Middle {
	/*!
	 * \brief Check the program for errors
	 * \return True if program is valid
	 */
	bool ErrorCheck::check(const IR::Program &program)
	{
		// Check each procedure of program
		for(const std::unique_ptr<IR::Procedure> &procedure : program.procedures()) {
			// *** Check for variables which are used before they are defined ***

			Analysis::Analysis analysis(*procedure);
			Analysis::LiveVariables liveVariables(*procedure, analysis.flowGraph());

			// Any variable which is live at the beginning of the function has no initial definition
			std::set<const IR::Symbol*> symbols = liveVariables.variables(procedure->entries().front());
			if(symbols.size() != 0) {
				// Return an error
				const IR::Symbol *symbol = *symbols.begin();
				std::stringstream s;
				s << "Use of undefined variable " << symbol->symbol->name;
				mErrorMessage = s.str();
				mErrorProcedure = procedure.get();
				return false;
			}

			// *** Check for control paths which do not return a value ***

			Analysis::FlowGraph flowGraph(*procedure);
			std::list<const Analysis::FlowGraph::Block*> blockList;
			std::set<const Analysis::FlowGraph::Block*> seenBlocks;
			Analysis::FlowGraph::Block *end = flowGraph.end();

			// Iterate backwards through the procedure, looking for any path which reaches the start label
			blockList.insert(blockList.begin(), end);
			for(const Analysis::FlowGraph::Block *block : blockList) {
				// Breadth-first search of the control flow graph
				if(seenBlocks.find(block) != seenBlocks.end()) {
					continue;
				}

				// Iterate backwards through the block.
				bool foundRet = false;
				for(const IR::Entry *entry : block->entries.reversed()) {
					if(entry->type == IR::Entry::Type::StoreRet) {
						// Found a StoreRet.  This block generates a return value
						foundRet = true;
						break;
					}
				}

				if(foundRet) {
					// Return value found.  No further walking necessary on this path
					continue;
				} else {
					// Continue scanning into the block's predecessors
					blockList.insert(blockList.end(), block->pred.begin(), block->pred.end());
				}

				if(block == flowGraph.start()) {
					// Made it to the beginning of the start block.  This means a path was found
					// which did not return a value.
					std::stringstream s;
					s << "Control reaches end of procedure without return";
					mErrorMessage = s.str();
					mErrorProcedure = procedure.get();
					return false;
				}

				seenBlocks.insert(block);
			}
		}

		return true;
	}
}