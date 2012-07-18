#include "ReachingDefs.h"

#include "Analysis/FlowGraph.h"

#include "IR/Entry.h"
#include "IR/Procedure.h"

#include "Util/UniqueQueue.h"

namespace Analysis {
	struct InOut {
		IR::EntrySet in;
		IR::EntrySet out;
	};

	static IR::EntrySet emptyEntrySet;

	ReachingDefs::ReachingDefs(IR::Procedure *procedure, FlowGraph &flowGraph)
		: mFlowGraph(flowGraph)
	{
		mProcedure = procedure;

		IR::EntrySet allDefs;
		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			if(entry->assign()) {
				allDefs.insert(entry);
			}
		}

		EntryToEntrySetMap gen;
		for(IR::EntryList::iterator it = procedure->entries().begin(); it != procedure->entries().end(); it++) {
			IR::Entry *entry = *it;
			if(entry->assign()) {
				gen[entry].insert(entry);
			}
		}

		EntryToEntrySetMap kill;
		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			if(!entry->assign()) {
				continue;
			}

			for(IR::EntrySet::const_iterator itDef = allDefs.begin(); itDef != allDefs.end(); itDef++) {
				IR::Entry *def = *itDef;
				if(def->assign() == entry->assign() && def != entry) {
					kill[entry].insert(def);
				}
			}
		}

		mDefs = analyze(mFlowGraph, gen, kill);
	}

	ReachingDefs::EntryToEntrySetMap ReachingDefs::analyze(FlowGraph &graph, EntryToEntrySetMap &gen, EntryToEntrySetMap &kill)
	{
		EntryToEntrySetMap map;
		std::map<FlowGraph::Block*, IR::EntrySet> genBlock;
		std::map<FlowGraph::Block*, IR::EntrySet> killBlock;

		for(FlowGraph::BlockSet::const_iterator it = graph.blocks().begin(); it != graph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;

			IR::EntrySet g;
			IR::EntrySet k;
			for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				g = transfer(g, gen[entry], kill[entry]);
				k = transfer(k, kill[entry], gen[entry]);
			}

			genBlock[block] = g;
			killBlock[block] = k;
		}

		std::map<FlowGraph::Block*, InOut> states;
		Util::UniqueQueue<FlowGraph::Block*> blockQueue;

		for(FlowGraph::BlockSet::const_iterator it = graph.blocks().begin(); it != graph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
			blockQueue.push(block);
		}

		while(!blockQueue.empty()) {
			FlowGraph::Block *block = blockQueue.front();
			blockQueue.pop();

			states[block].in.clear();
			for(FlowGraph::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
				FlowGraph::Block *pred = *it;
				IR::EntrySet &out = states[pred].out;
				states[block].in.insert(out.begin(), out.end());
			}

			IR::EntrySet out = transfer(states[block].in, genBlock[block], killBlock[block]);

			if(out != states[block].out) {
				states[block].out = out;
				for(FlowGraph::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
					FlowGraph::Block *succ = *it;
					blockQueue.push(succ);
				}
			}
		}

		for(FlowGraph::BlockSet::iterator it = graph.blocks().begin(); it != graph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
			IR::EntrySet set = states[block].in;
			for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				map[entry] = set;
				set = transfer(set, gen[entry], kill[entry]);
			}
		}

		return map;
	}

	const IR::EntrySet &ReachingDefs::defs(IR::Entry *entry) const
	{
		std::map<IR::Entry*, IR::EntrySet>::const_iterator it = mDefs.find(entry);
		if(it != mDefs.end()) {
			return it->second;
		} else {
			return emptyEntrySet;
		}
	}

	const IR::EntrySet ReachingDefs::defsForSymbol(IR::Entry *entry, IR::Symbol *symbol) const
	{
		IR::EntrySet result;
		const IR::EntrySet &all = defs(entry);
		for(IR::EntrySet::const_iterator it = all.begin(); it != all.end(); it++) {
			IR::Entry *def = *it;
			if(def->assign() == symbol) {
				result.insert(def);
			}
		}

		return result;
	}

	void ReachingDefs::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		mDefs[newEntry] = mDefs[oldEntry];
		mDefs.erase(oldEntry);
		for(EntryToEntrySetMap::iterator it = mDefs.begin(); it != mDefs.end(); it++) {
			IR::EntrySet &set = it->second;
			IR::EntrySet::iterator setIt = set.find(oldEntry);
			if(setIt != set.end()) {
				set.erase(setIt);
				set.insert(newEntry);
			}
		}
	}

	void ReachingDefs::remove(IR::Entry *entry)
	{
		for(EntryToEntrySetMap::iterator it = mDefs.begin(); it != mDefs.end(); it++) {
			IR::EntrySet &set = it->second;
			IR::EntrySet::iterator setIt = set.find(entry);
			if(setIt != set.end()) {
				set.erase(setIt);
			}
		}
		mDefs.erase(entry);
	}

	void ReachingDefs::print() const
	{
		int line = 1;
		std::map<IR::Entry*, int> lineMap;

		for(IR::EntryList::iterator itEntry = mProcedure->entries().begin(); itEntry != mProcedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			lineMap[entry] = line;
			printf("%i: ", line);
			entry->print();
			printf(" -> ");
			IR::EntrySet d = defs(entry);
			for(IR::EntrySet::iterator it2 = d.begin(); it2 != d.end(); it2++) {
				IR::Entry *e = *it2;
				printf("%i ", lineMap[e]);
			}
			printf("\n");
			line++;
		}
	}

	IR::EntrySet ReachingDefs::transfer(const IR::EntrySet &in, const IR::EntrySet &gen, const IR::EntrySet &kill)
	{
		IR::EntrySet out(gen.begin(), gen.end());

		for(IR::EntrySet::const_iterator itIn = in.begin(); itIn != in.end(); itIn++) {
			IR::Entry *entryIn = *itIn;
			if(kill.find(entryIn) == kill.end()) {
				out.insert(entryIn);
			}
		}

		return out;
	}
}