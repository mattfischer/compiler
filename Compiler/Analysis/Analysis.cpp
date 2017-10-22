#include "Analysis/Analysis.h"

namespace Analysis {
	Analysis::Analysis(const IR::Procedure &procedure)
		: mProcedure(procedure)
	{
	}

	const FlowGraph &Analysis::flowGraph()
	{
		if(!mFlowGraph) {
			mFlowGraph = std::make_unique<FlowGraph>(mProcedure);
		}

		return *mFlowGraph;
	}

	const ReachingDefs &Analysis::reachingDefs()
	{
		if(!mReachingDefs) {
			mReachingDefs = std::make_unique<ReachingDefs>(mProcedure, flowGraph());
		}

		return *mReachingDefs;
	}

	const UseDefs &Analysis::useDefs()
	{
		if(!mUseDefs) {
			mUseDefs = std::make_unique<UseDefs>(mProcedure, reachingDefs());
		}

		return *mUseDefs;
	}

	const Constants &Analysis::constants()
	{
		if(!mConstants) {
			mConstants = std::make_unique<Constants>(mProcedure, useDefs());
		}

		return *mConstants;
	}

	void Analysis::invalidate()
	{
		mConstants.reset();
		mUseDefs.reset();
		mReachingDefs.reset();
		mFlowGraph.reset();
	}

	void Analysis::replace(IR::Entry *oldEntry, IR::Entry *newEntry)
	{
		if(mUseDefs) {
			mUseDefs->replace(oldEntry, newEntry);
		}

		if(mReachingDefs) {
			mReachingDefs->replace(oldEntry, newEntry);
		}
	}

	void Analysis::replaceUse(IR::Entry *entry, const IR::Symbol *oldSymbol, const IR::Symbol *newSymbol)
	{
		if(mUseDefs) {
			mUseDefs->replaceUse(entry, oldSymbol, newSymbol);
		}
	}

	void Analysis::remove(IR::Entry *entry)
	{
		if(mUseDefs) {
			mUseDefs->remove(entry);
		}

		if(mReachingDefs) {
			mReachingDefs->remove(entry);
		}
	}
}