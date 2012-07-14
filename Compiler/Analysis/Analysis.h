#ifndef ANALYSIS_ANALYSIS_H
#define ANALYSIS_ANALYSIS_H

#include "Analysis/BlockSort.h"
#include "Analysis/DominanceFrontiers.h"
#include "Analysis/DominatorTree.h"
#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"

namespace IR {
	class Procedure;
	class Entry;
	class Symbol;
}

namespace Analysis {
	class Analysis {
	public:
		Analysis(IR::Procedure *procedure);

		BlockSort &blockSort() { return mBlockSort; }
		DominanceFrontiers &dominanceFrontiers() { return mDominanceFrontiers; }
		DominatorTree &dominatorTree() { return mDominatorTree; }
		FlowGraph &flowGraph() { return mFlowGraph; }
		ReachingDefs &reachingDefs() { return mReachingDefs; }
		UseDefs &useDefs() { return mUseDefs; }

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol);
		void remove(IR::Entry *entry);
	private:
		FlowGraph mFlowGraph;
		BlockSort mBlockSort;
		DominatorTree mDominatorTree;
		DominanceFrontiers mDominanceFrontiers;
		ReachingDefs mReachingDefs;
		UseDefs mUseDefs;
	};
}
#endif