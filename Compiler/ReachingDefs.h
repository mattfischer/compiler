#ifndef REACHING_DEFS_H
#define REACHING_DEFS_H

#include "IR.h"

#include <vector>
#include <set>
#include <map>

class ReachingDefs {
public:
	ReachingDefs(std::vector<IR::Block*> &blocks);

	std::set<IR::Entry*> &defs(IR::Entry* entry);
	void print();

private:
	std::set<IR::Entry*> createKillSet(std::set<IR::Entry*> &in, std::set<IR::Entry*> &gen);
	std::set<IR::Entry*> createOutSet(std::set<IR::Entry*> &in, std::set<IR::Entry*> &gen, std::set<IR::Entry*> &kill);

	std::vector<IR::Block*> mBlocks;
	std::map<IR::Entry*, std::set<IR::Entry*> > mDefs;
};
#endif