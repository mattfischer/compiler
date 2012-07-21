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

		enum MeetType {
			MeetTypeUnion,
			MeetTypeIntersect
		};

		enum Direction {
			DirectionForward,
			DirectionBackward
		};

		ItemToItemSetMap analyze(FlowGraph &graph, ItemToItemSetMap &gen, ItemToItemSetMap &kill, ItemSet &all, MeetType meetType, Direction direction)
		{
			ItemToItemSetMap map;
			std::map<FlowGraph::Block*, ItemSet> genBlock;
			std::map<FlowGraph::Block*, ItemSet> killBlock;

			for(FlowGraph::BlockSet::const_iterator it = graph.blocks().begin(); it != graph.blocks().end(); it++) {
				FlowGraph::Block *block = *it;

				ItemSet g;
				ItemSet k;
				switch(direction) {
					case DirectionForward:
						for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
							IR::Entry *entry = *itEntry;
							g = transfer(g, gen[entry], kill[entry]);
							k = transfer(k, kill[entry], gen[entry]);
						}
						break;

					case DirectionBackward:
						for(IR::EntryList::reverse_iterator itEntry = block->entries.rbegin(); itEntry != block->entries.rend(); itEntry++) {
							IR::Entry *entry = *itEntry;
							g = transfer(g, gen[entry], kill[entry]);
							k = transfer(k, kill[entry], gen[entry]);
						}
						break;
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

				switch(meetType) {
					case MeetTypeUnion:
						states[block].out.clear();
						break;

					case MeetTypeIntersect:
						states[block].out = all;
						break;
				}
			}

			while(!blockQueue.empty()) {
				FlowGraph::Block *block = blockQueue.front();
				blockQueue.pop();

				switch(meetType) {
					case MeetTypeUnion:
						states[block].in.clear();
						break;

					case MeetTypeIntersect:
						if((direction == DirectionForward && block == graph.start()) ||
							(direction == DirectionBackward && block == graph.end())) {
							states[block].in.clear();
						} else {
							states[block].in = all;
						}
				}

				switch(direction) {
					case DirectionForward:
						for(FlowGraph::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
							FlowGraph::Block *pred = *it;
							ItemSet &out = states[pred].out;
							states[block].in = meet(states[block].in, states[pred].out, meetType);
						}
						break;

					case DirectionBackward:
						for(FlowGraph::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
							FlowGraph::Block *succ = *it;
							ItemSet &out = states[succ].out;
							states[block].in = meet(states[block].in, states[succ].out, meetType);
						}
						break;
				}

				ItemSet out = transfer(states[block].in, genBlock[block], killBlock[block]);

				if(out != states[block].out) {
					states[block].out = out;
					switch(direction) {
						case DirectionForward:
							for(FlowGraph::BlockSet::iterator it = block->succ.begin(); it != block->succ.end(); it++) {
								FlowGraph::Block *succ = *it;
								blockQueue.push(succ);
							}
							break;

						case DirectionBackward:
							for(FlowGraph::BlockSet::iterator it = block->pred.begin(); it != block->pred.end(); it++) {
								FlowGraph::Block *pred = *it;
								blockQueue.push(pred);
							}
							break;
					}
				}
			}

			for(FlowGraph::BlockSet::iterator it = graph.blocks().begin(); it != graph.blocks().end(); it++) {
				FlowGraph::Block *block = *it;
				ItemSet set = states[block].in;
				switch(direction) {
					case DirectionForward:
						for(IR::EntryList::iterator itEntry = block->entries.begin(); itEntry != block->entries.end(); itEntry++) {
							IR::Entry *entry = *itEntry;
							map[entry] = set;
							set = transfer(set, gen[entry], kill[entry]);
						}
						break;

					case DirectionBackward:
						for(IR::EntryList::reverse_iterator itEntry = block->entries.rbegin(); itEntry != block->entries.rend(); itEntry++) {
							IR::Entry *entry = *itEntry;
							map[entry] = set;
							set = transfer(set, gen[entry], kill[entry]);
						}
						break;
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

		ItemSet meet(ItemSet &a, ItemSet &b, MeetType meetType)
		{
			ItemSet out;

			switch(meetType) {
				case MeetTypeUnion:
					out.insert(a.begin(), a.end());
					out.insert(b.begin(), b.end());
					break;

				case MeetTypeIntersect:
					for(ItemSet::iterator it = a.begin(); it != a.end(); it++) {
						Item item = *it;
						if(b.find(item) != b.end()) {
							out.insert(item);
						}
					}
					break;
			}

			return out;
		}
	};
}
#endif