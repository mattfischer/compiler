#include "Analysis/Analysis.h"

namespace Analysis {
	Analysis::Analysis(IR::Procedure *procedure)
	{
		mProcedure = procedure;

		mFlowGraph = 0;
		mReachingDefs = 0;
		mUseDefs = 0;
		mConstants = 0;
	}

	Analysis::~Analysis()
	{
		if(mConstants) {
			delete mConstants;
		}

		if(mUseDefs) {
			delete mUseDefs;
		}

		if(mReachingDefs) {
			delete mReachingDefs;
		}

		if(mFlowGraph) {
			delete mFlowGraph;
		}
	}

	FlowGraph *Analysis::flowGraph()
	{
		if(!mFlowGraph) {
			mFlowGraph = new FlowGraph(mProcedure);
		}

		return mFlowGraph;
	}

	ReachingDefs *Analysis::reachingDefs()
	{
		if(!mReachingDefs) {
			mReachingDefs = new ReachingDefs(mProcedure, flowGraph());
		}

		return mReachingDefs;
	}

	UseDefs *Analysis::useDefs()
	{
		if(!mUseDefs) {
			mUseDefs = new UseDefs(mProcedure, reachingDefs());
		}

		return mUseDefs;
	}

	Constants *Analysis::constants()
	{
		if(!mConstants) {
			mConstants = new Constants(mProcedure, useDefs());
		}

		return mConstants;
	}

	void Analysis::invalidate()
	{
		if(mConstants) {
			delete mConstants;
			mConstants = 0;
		}

		if(mUseDefs) {
			delete mUseDefs;
			mUseDefs = 0;
		}

		if(mReachingDefs) {
			delete mReachingDefs;
			mReachingDefs = 0;
		}

		if(mFlowGraph) {
			delete mFlowGraph;
			mFlowGraph = 0;
		}
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

	void Analysis::replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol)
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