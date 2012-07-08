#ifndef REACHING_DEFS_H
#define REACHING_DEFS_H

#include "IR.h"

#include <vector>
#include <set>
#include <map>

class ReachingDefs {
public:
	ReachingDefs(const std::vector<IR::Block*> &blocks);

	typedef std::set<IR::Entry*> EntrySet;
	const EntrySet &defs(IR::Entry* entry) const;
	void print() const;

private:
	EntrySet createKillSet(const EntrySet &in, const EntrySet &gen);
	EntrySet createOutSet(const EntrySet &in, const EntrySet &gen, const EntrySet &kill);

	std::vector<IR::Block*> mBlocks;
	std::map<IR::Entry*, EntrySet> mDefs;
};
#endif