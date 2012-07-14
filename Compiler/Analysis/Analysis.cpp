#include "Analysis/Analysis.h"

#include "IR/Procedure.h"

namespace Analysis {
	Analysis::Analysis(IR::Procedure *procedure)
		: mFlowGraph(procedure), mReachingDefs(procedure, mFlowGraph),
		  mUseDefs(procedure, mReachingDefs), mDominatorTree(mFlowGraph),
		  mDominanceFrontiers(mDominatorTree), mBlockSort(mFlowGraph)
	{
	}

	void Analysis::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		mFlowGraph.replace(oldEntry, newEntry);
		mReachingDefs.replace(oldEntry, newEntry);
		mUseDefs.replace(oldEntry, newEntry);
	}

	void Analysis::replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol)
	{
		mUseDefs.replaceUse(entry, oldSymbol, newSymbol);
	}

	void Analysis::remove(IR::Entry *entry)
	{
		mReachingDefs.remove(entry);
		mUseDefs.remove(entry);
	}
}