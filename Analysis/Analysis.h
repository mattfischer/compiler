#ifndef ANALYSIS_ANALYSIS_H
#define ANALYSIS_ANALYSIS_H

#include "Analysis/FlowGraph.h"
#include "Analysis/ReachingDefs.h"
#include "Analysis/UseDefs.h"
#include "Analysis/Constants.h"

#include "IR/Procedure.h"

#include <memory>

namespace Analysis {
	class Analysis {
	public:
		Analysis(const IR::Procedure &procedure);

		const FlowGraph &flowGraph();
		const ReachingDefs &reachingDefs();
		const UseDefs &useDefs();
		const Constants &constants();

		void invalidate();

		void replace(IR::Entry *oldEntry, IR::Entry *newEntry);
		void replaceUse(IR::Entry *entry, const IR::Symbol *oldSymbol, const IR::Symbol *newSymbol);
		void remove(const IR::Entry *entry);

	private:
		std::unique_ptr<FlowGraph> mFlowGraph;
		std::unique_ptr<ReachingDefs> mReachingDefs;
		std::unique_ptr<UseDefs> mUseDefs;
		std::unique_ptr<Constants> mConstants;

		const IR::Procedure &mProcedure;
	};
}

#endif