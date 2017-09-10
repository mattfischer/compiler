#ifndef ANALYSIS_AVAILABLE_EXPRESSIONS_H
#define ANALYSIS_AVAILABLE_EXPRESSIONS_H

#include "IR/Procedure.h"
#include "IR/Entry.h"

#include "Analysis/FlowGraph.h"

#include <map>
#include <iostream>

namespace Analysis {
	/*!
	 * \brief Determine the set of expressions available at each point in a procedure
	 *
	 * An expression is available if none of the symbols it uses have been assigned to
	 * since its definition.  If an expression is available, it is a candidate for
	 * common subexpression elimination.
	 */
	class AvailableExpressions {
	public:
		AvailableExpressions(const IR::Procedure &procedure, FlowGraph &flowGraph);

		const std::set<IR::Entry*> &expressions(IR::Entry *entry) const;

		void print(std::ostream &o) const;

		static bool isExpression(IR::Entry *entry);

	private:
		IR::Procedure &mProcedure;

		std::map<IR::Entry*, std::set<IR::Entry*>> mExpressions;
	};
}

#endif