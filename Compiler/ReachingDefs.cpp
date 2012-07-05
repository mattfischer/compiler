#include "ReachingDefs.h"

#include <queue>

struct InOut {
	std::set<IR::Entry*> in;
	std::set<IR::Entry*> out;
};

ReachingDefs::ReachingDefs(std::vector<IR::Block*> &blocks)
{
	mBlocks = blocks;

	std::map<IR::Block*, InOut> states;
	std::map<IR::Block*, std::set<IR::Entry*> > gen;
	std::queue<IR::Block*> blockQueue;

	for(unsigned int i=0; i<blocks.size(); i++) {
		IR::Block *block = blocks[i];
		std::set<IR::Entry*> out;
		for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
			if(!entry->assignSymbol()) {
				continue;
			}

			std::set<IR::Entry*> g;
			g.insert(entry);

			std::set<IR::Entry*> k = createKillSet(out, g);
			out = createOutSet(out, g, k);
		}
		gen[block] = out;
	}

	for(unsigned int i=0; i<blocks.size(); i++) {
		blockQueue.push(blocks[i]);
	}

	while(!blockQueue.empty()) {
		IR::Block *block = blockQueue.front();
		blockQueue.pop();

		states[block].in.clear();
		for(unsigned int i=0; i<block->pred.size(); i++) {
			IR::Block *pred = block->pred[i];
			std::set<IR::Entry*> &out = states[pred].out;
			states[block].in.insert(out.begin(), out.end());
		}

		std::set<IR::Entry*> kill = createKillSet(states[block].in, gen[block]);
		std::set<IR::Entry*> newOut = createOutSet(states[block].in, gen[block], kill);

		if(newOut != states[block].out) {
			states[block].out = newOut;
			for(unsigned int i=0; i<block->succ.size(); i++) {
				blockQueue.push(block->succ[i]);
			}
		}
	}

	for(unsigned int i=0; i<blocks.size(); i++) {
		IR::Block *block = blocks[i];
		std::set<IR::Entry*> out = states[block].in;
		for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
			mDefs[entry] = out;

			if(entry->assignSymbol()) {
				std::set<IR::Entry*> g;
				g.insert(entry);

				std::set<IR::Entry*> k = createKillSet(out, g);
				out = createOutSet(out, g, k);
			}
		}
	}
}

std::set<IR::Entry*> &ReachingDefs::defs(IR::Entry *entry)
{
	return mDefs[entry];
}

void ReachingDefs::print()
{
	int line = 1;
	std::map<IR::Entry*, int> lineMap;
	for(int i=0; i<mBlocks.size(); i++) {
		IR::Block *block = mBlocks[i];
		for(IR::Entry *entry = block->head()->next; entry != block->tail(); entry = entry->next) {
			lineMap[entry] = line;
			printf("%i: ", line);
			entry->print();
			printf(" -> ");
			std::set<IR::Entry*> d = defs(entry);
			for(std::set<IR::Entry*>::iterator it = d.begin(); it != d.end(); it++) {
				IR::Entry *e = *it;
				printf("%i ", lineMap[e]);
			}
			printf("\n");
			line++;
		}
	}
}

std::set<IR::Entry*> ReachingDefs::createKillSet(std::set<IR::Entry*> &in, std::set<IR::Entry*> &gen)
{
	std::set<IR::Entry*> kill;
	for(std::set<IR::Entry*>::iterator it = in.begin(); it != in.end(); it++) {
		IR::Entry *entry = *it;
		for(std::set<IR::Entry*>::iterator it2 = gen.begin(); it2 != gen.end(); it2++) {
			IR::Entry *genEntry = *it2;
			if(genEntry->assignSymbol() == entry->assignSymbol()) {
				kill.insert(entry);
			}
		}
	}

	return kill;
}

std::set<IR::Entry*> ReachingDefs::createOutSet(std::set<IR::Entry*> &in, std::set<IR::Entry*> &gen, std::set<IR::Entry*> &kill)
{
	std::set<IR::Entry*> out(gen.begin(), gen.end());
	for(std::set<IR::Entry*>::iterator it = in.begin(); it != in.end(); it++) {
		IR::Entry *entry = *it;
		if(kill.find(entry) == kill.end()) {
			out.insert(entry);
		}
	}

	return out;
}