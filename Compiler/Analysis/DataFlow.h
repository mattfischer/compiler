#ifndef ANALYSIS_DATA_FLOW_H
#define ANALYSIS_DATA_FLOW_H

#include "Analysis/FlowGraph.h"

#include "Util/UniqueQueue.h"

#include "IR/Entry.h"

#include <map>
#include <set>

namespace Analysis {
	/*!
	 * \brief Implements the data flow analysis algorithm
	 *
	 * The data flow algorithm is a generic method of propagating information around a
	 * control flow graph.  It can be used for many different analyses, such as reaching
	 * definitions, live variable analysis, etc.
	 *
	 * A data flow analysis works with Items, which is a generic term for some type of data
	 * that can be associated with an IR entry.  The output of the analysis is the list of
	 * items associated with each entry in the procedure.
	 *
	 * The components of a data flow analysis are:
	 *
	 * gen - The set of items created by each entry in the procedure.  The items are then
	 *       propagated forward (or backward, depending on direction) through the graph, and
	 *       into successor (or predecessor) nodes in the control flow graph
	 * kill - The set of items removed by each entry from the current item set.
	 * direction - Whether items flow forward or backward through the graph
	 * meet - How the item sets from the outputs of multiple blocks are combined to form the
	 *        input of a block which they all feed into.  The meet operation can be either
	 *        set union, or set intersection.
	 *
	 * The data flow algorithm takes in these parameters, and propagates items around the control
	 * flow graph until no more changes are made.
	 */
	template<typename T>
	class DataFlow {
	public:
		typedef T Item;
		typedef std::set<Item> ItemSet;
		typedef std::map<IR::Entry*, ItemSet> EntryToItemSetMap;

		/*!
		 * \brief Which operation to use when meeting blocks
		 */
		enum class Meet {
			Union, //!< Set union
			Intersect //!< Set intersection
		};

		/*!
		 * \brief Direction that items flow through the graph
		 */
		enum class Direction {
			Forward, //!< Items flow forward
			Backward //!< Items flow backward
		};

		/*!
		 * \brief Analyze a control flow graph
		 * \param graph Graph to analyze
		 * \param gen Gen sets
		 * \param kill Kill sets
		 * \param all All items that can occur in the output sets
		 * \param meetType Operation to use when meeting edges
		 * \param direction Direction of data flow
		 * \return Set of items attached to each entry of the graph
		 */
		EntryToItemSetMap analyze(FlowGraph &graph, EntryToItemSetMap &gen, EntryToItemSetMap &kill, ItemSet &all, Meet meetType, Direction direction)
		{
			EntryToItemSetMap map;
			std::map<FlowGraph::Block*, ItemSet> genBlock;
			std::map<FlowGraph::Block*, ItemSet> killBlock;

			// Aggregate the gen/kill sets from each entry into gen/kill sets for each block
			for(std::unique_ptr<FlowGraph::Block> &block : graph.blocks()) {
				ItemSet g;
				ItemSet k;
				switch(direction) {
					case Direction::Forward:
						for(IR::Entry *entry : block->entries) {
							g = transfer(g, gen[entry], kill[entry]);
							k = transfer(k, kill[entry], gen[entry]);
						}
						break;

					case Direction::Backward:
						for(IR::EntryList::reverse_iterator itEntry = block->entries.rbegin(); itEntry != block->entries.rend(); itEntry++) {
							IR::Entry *entry = *itEntry;
							g = transfer(g, gen[entry], kill[entry]);
							k = transfer(k, kill[entry], gen[entry]);
						}
						break;
				}

				genBlock[block.get()] = g;
				killBlock[block.get()] = k;
			}

			struct InOut {
				ItemSet in;
				ItemSet out;
			};

			std::map<FlowGraph::Block*, InOut> states;
			Util::UniqueQueue<FlowGraph::Block*> blockQueue;

			// Populate the initial states of each block, based on meet type
			for(std::unique_ptr<FlowGraph::Block> &block : graph.blocks()) {
				blockQueue.push(block.get());

				switch(meetType) {
					case Meet::Union:
						states[block.get()].out.clear();
						break;

					case Meet::Intersect:
						states[block.get()].out = all;
						break;
				}
			}

			// The core of the algorithm.  Process blocks until there are no more to process
			while(!blockQueue.empty()) {
				// Grab front block from queue
				FlowGraph::Block *block = blockQueue.front();
				blockQueue.pop();

				// Construct the results of the meet operation.  First, initialize state
				switch(meetType) {
					case Meet::Union:
						states[block].in.clear();
						break;

					case Meet::Intersect:
						if((direction == Direction::Forward && block == graph.start()) ||
							(direction == Direction::Backward && block == graph.end())) {
							states[block].in.clear();
						} else {
							states[block].in = all;
						}
				}

				// Now examine each predecessor/successor, and union/intersect their state into the
				// current block's state
				switch(direction) {
					case Direction::Forward:
						for(FlowGraph::Block *pred : block->pred) {
							ItemSet &out = states[pred].out;
							states[block].in = meet(states[block].in, states[pred].out, meetType);
						}
						break;

					case Direction::Backward:
						for(FlowGraph::Block *succ : block->succ) {
							ItemSet &out = states[succ].out;
							states[block].in = meet(states[block].in, states[succ].out, meetType);
						}
						break;
				}

				// Now that the block's input state has been calculated, apply the gen/kill sets
				// to determine the block's output state
				ItemSet out = transfer(states[block].in, genBlock[block], killBlock[block]);

				// If any changes were made to the block's state, add all of its predecessors/successors
				// to the queue for further processing
				if(out != states[block].out) {
					states[block].out = out;
					switch(direction) {
						case Direction::Forward:
							for(FlowGraph::Block *succ : block->succ) {
								blockQueue.push(succ);
							}
							break;

						case Direction::Backward:
							for(FlowGraph::Block *pred : block->pred) {
								blockQueue.push(pred);
							}
							break;
					}
				}
			}

			// The core algorithm is now complete--each block has information about its starting set
			// of items.  Now proceed through each block, using the state information and the gen/kill
			// sets to assign an item set to each entry within the block
			for(std::unique_ptr<FlowGraph::Block> &block : graph.blocks()) {
				ItemSet set = states[block.get()].in;
				switch(direction) {
					case Direction::Forward:
						for(IR::Entry *entry : block->entries) {
							map[entry] = set;
							set = transfer(set, gen[entry], kill[entry]);
						}
						break;

					case Direction::Backward:
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
		/*!
		 * \brief Apply gen/kill sets to an input set to create an output set
		 * \param in Input set
		 * \param gen Gen set
		 * \param kill Kill set
		 * \return Output set
		 */
		ItemSet transfer(const ItemSet &in, const ItemSet &gen, const ItemSet &kill)
		{
			// Out set begins with all gen entries
			ItemSet out(gen.begin(), gen.end());

			// Add any entry from the input set which is not in the kill set
			for(ItemSet::const_iterator itIn = in.begin(); itIn != in.end(); itIn++) {
				Item item = *itIn;
				if(kill.find(item) == kill.end()) {
					out.insert(item);
				}
			}

			return out;
		}

		/*!
		 * \brief Calculate the result of two sets meeting
		 * \param a Input set
		 * \param b Input set
		 * \param meetType Type of meet algorithm to use
		 * \return Results of meet operation
		 */
		ItemSet meet(ItemSet &a, ItemSet &b, Meet meetType)
		{
			ItemSet out;

			switch(meetType) {
				case Meet::Union:
					out.insert(a.begin(), a.end());
					out.insert(b.begin(), b.end());
					break;

				case Meet::Intersect:
					for(Item item : a) {
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