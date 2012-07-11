#include "ReachingDefs.h"

#include "Analysis/FlowGraph.h"

#include "IR/Entry.h"
#include "IR/Block.h"

#include <queue>

namespace Analysis {
	struct InOut {
		ReachingDefs::EntrySet in;
		ReachingDefs::EntrySet out;
	};

	static ReachingDefs::EntrySet emptyEntrySet;

	ReachingDefs::ReachingDefs(FlowGraph &flowGraph)
		: mFlowGraph(flowGraph)
	{
		std::map<FlowGraph::Block*, InOut> states;
		std::map<FlowGraph::Block*, EntrySet> gen;
		std::queue<FlowGraph::Block*> blockQueue;

		for(FlowGraph::BlockSet::const_iterator it = mFlowGraph.blocks().begin(); it != mFlowGraph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
			IR::Block* irBlock = block->irBlock;
			EntrySet out;
			for(IR::EntryList::iterator itEntry = irBlock->entries.begin(); itEntry != irBlock->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				if(!entry->assignSymbol()) {
					continue;
				}

				EntrySet g;
				g.insert(entry);

				EntrySet k = createKillSet(out, g);
				out = createOutSet(out, g, k);
			}
			gen[block] = out;
		}

		for(FlowGraph::BlockSet::const_iterator it = mFlowGraph.blocks().begin(); it != mFlowGraph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
			blockQueue.push(block);
		}

		while(!blockQueue.empty()) {
			FlowGraph::Block *block = blockQueue.front();
			blockQueue.pop();

			states[block].in.clear();
			for(FlowGraph::Block::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
				FlowGraph::Block *pred = *it;
				EntrySet &out = states[pred].out;
				states[block].in.insert(out.begin(), out.end());
			}

			EntrySet kill = createKillSet(states[block].in, gen[block]);
			EntrySet newOut = createOutSet(states[block].in, gen[block], kill);

			if(newOut != states[block].out) {
				states[block].out = newOut;
				for(FlowGraph::Block::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
					FlowGraph::Block *succ = *it;
					blockQueue.push(succ);
				}
			}
		}

		for(FlowGraph::BlockSet::iterator it = mFlowGraph.blocks().begin(); it != mFlowGraph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
			IR::Block *irBlock = block->irBlock;
			EntrySet out = states[block].in;
			for(IR::EntryList::iterator itEntry = irBlock->entries.begin(); itEntry != irBlock->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				mDefs[entry] = out;

				if(entry->assignSymbol()) {
					EntrySet g;
					g.insert(entry);

					EntrySet k = createKillSet(out, g);
					out = createOutSet(out, g, k);
				}
			}
		}
	}

	const ReachingDefs::EntrySet &ReachingDefs::defs(IR::Entry *entry) const
	{
		std::map<IR::Entry*, EntrySet>::const_iterator it = mDefs.find(entry);
		if(it != mDefs.end()) {
			return it->second;
		} else {
			return emptyEntrySet;
		}
	}

	const ReachingDefs::EntrySet ReachingDefs::defsForSymbol(IR::Entry *entry, IR::Symbol *symbol) const
	{
		EntrySet result;
		const EntrySet &all = defs(entry);
		for(EntrySet::const_iterator it = all.begin(); it != all.end(); it++) {
			IR::Entry *def = *it;
			if(def->assignSymbol() == symbol) {
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
			EntrySet &set = it->second;
			EntrySet::iterator setIt = set.find(oldEntry);
			if(setIt != set.end()) {
				set.erase(setIt);
				set.insert(newEntry);
			}
		}
	}

	void ReachingDefs::remove(IR::Entry *entry)
	{
		for(EntryToEntrySetMap::iterator it = mDefs.begin(); it != mDefs.end(); it++) {
			EntrySet &set = it->second;
			EntrySet::iterator setIt = set.find(entry);
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
		for(FlowGraph::BlockSet::iterator it = mFlowGraph.blocks().begin(); it != mFlowGraph.blocks().end(); it++) {
			FlowGraph::Block *block = *it;
			IR::Block *irBlock = block->irBlock;
			for(IR::EntryList::iterator itEntry = irBlock->entries.begin(); itEntry != irBlock->entries.end(); itEntry++) {
				IR::Entry *entry = *itEntry;
				lineMap[entry] = line;
				printf("%i: ", line);
				entry->print();
				printf(" -> ");
				EntrySet d = defs(entry);
				for(EntrySet::iterator it2 = d.begin(); it2 != d.end(); it++) {
					IR::Entry *e = *it2;
					printf("%i ", lineMap[e]);
				}
				printf("\n");
				line++;
			}
		}
	}

	ReachingDefs::EntrySet ReachingDefs::createKillSet(const EntrySet &in, const EntrySet &gen)
	{
		EntrySet kill;
		for(EntrySet::const_iterator it = in.begin(); it != in.end(); it++) {
			IR::Entry *entry = *it;
			for(EntrySet::const_iterator it2 = gen.begin(); it2 != gen.end(); it2++) {
				IR::Entry *genEntry = *it2;
				if(genEntry->assignSymbol() == entry->assignSymbol()) {
					kill.insert(entry);
				}
			}
		}

		return kill;
	}

	ReachingDefs::EntrySet ReachingDefs::createOutSet(const EntrySet &in, const EntrySet &gen, const EntrySet &kill)
	{
		EntrySet out(gen.begin(), gen.end());
		for(EntrySet::const_iterator it = in.begin(); it != in.end(); it++) {
			IR::Entry *entry = *it;
			if(kill.find(entry) == kill.end()) {
				out.insert(entry);
			}
		}

		return out;
	}
}