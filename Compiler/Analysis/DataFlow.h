#ifndef ANALYSIS_DATA_FLOW_H
#define ANALYSIS_DATA_FLOW_H

#include "Analysis/FlowGraph.h"

#include "Util/UniqueQueue.h"

#include <map>
#include <set>

namespace Analysis {
	template<typename T>
	class DataFlow {
	public:
		typedef T Item;
		typedef std::set<Item> ItemSet;
		typedef std::map<Item, ItemSet> ItemToItemSetMap;

		ItemToItemSetMap analyze(FlowGraph &graph, ItemToItemSetMap &gen, ItemToItemSetMap &kill)
		{
			ItemToItemSetMap map;
			std::map<FlowGraph::Block*, ItemSet> genBlock;
			std::map<FlowGraph::Block*, ItemSet> killBlock;

			for(FlowGraph::BlockSet::const_iterator it = graph.blocks().begin(); it != graph.blocks().end(); it++) {
				FlowGraph::Block *block = *it;

				ItemSet g;
				ItemSet k;
				for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
					IR::Entry *entry = *itEntry;
					g = transfer(g, gen[entry], kill[entry]);
					k = transfer(k, kill[entry], gen[entry]);
				}

				genBlock[block] = g;
				killBlock[block] = k;
			}

			struct InOut {
				ItemSet in;
				ItemSet out;
			};

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
					ItemSet &out = states[pred].out;
					states[block].in.insert(out.begin(), out.end());
				}

				ItemSet out = transfer(states[block].in, genBlock[block], killBlock[block]);

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
				ItemSet set = states[block].in;
				for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
					IR::Entry *entry = *itEntry;
					map[entry] = set;
					set = transfer(set, gen[entry], kill[entry]);
				}
			}

			return map;
		}

	private:
		ItemSet transfer(const ItemSet &in, const ItemSet &gen, const ItemSet &kill)
		{
			ItemSet out(gen.begin(), gen.end());

			for(ItemSet::const_iterator itIn = in.begin(); itIn != in.end(); itIn++) {
				Item item = *itIn;
				if(kill.find(item) == kill.end()) {
					out.insert(item);
				}
			}

			return out;
		}
	};
}
#endif