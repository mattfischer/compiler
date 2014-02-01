#ifndef ANALYSIS_ANALYSIS_H
#define ANALYSIS_ANALYSIS_H

#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"
#include "Analysis/Constants.h"

#include "IR/Procedure.h"

namespace Analysis {
	class Analysis {
	public:
		Analysis(IR::Procedure *procedure);
		~Analysis();

		FlowGraph *flowGraph();
		ReachingDefs *reachingDefs();
		UseDefs *useDefs();
		Constants *constants();

		void invalidate();

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void replaceUse(IR::Entry *entry, IR::Symbol *oldSymbol, IR::Symbol *newSymbol);
		void remove(IR::Entry *entry);

	private:
		FlowGraph *mFlowGraph;
		ReachingDefs *mReachingDefs;
		UseDefs *mUseDefs;
		Constants *mConstants;

		IR::Procedure *mProcedure;
	};
}

#endif