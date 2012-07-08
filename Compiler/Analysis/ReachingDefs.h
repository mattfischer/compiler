#ifndef ANALYSIS_REACHING_DEFS_H
#define ANALYSIS_REACHING_DEFS_H

#include <vector>
#include <set>
#include <map>

namespace IR {
	struct Block;
	struct Entry;
}

namespace Analysis {
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
}
#endif