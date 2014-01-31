#include "Analysis/Constants.h"

namespace Analysis {
	Constants::Constants(IR::Procedure *procedure)
		: mUseDefs(procedure)
	{
	}

	/*!
	 * \brief Get the value of a symbol at an entry, if it can be determined to be a constant
	 * \param entry Entry to examine
	 * \param symbol Symbol to evaluate
	 * \param useDefs Use-def chains for procedure
	 * \param isConstant [out] True if symbol has a constant value, false otherwise
	 * \return Value of symbol
	 */
	int Constants::getValue(IR::Entry *entry, IR::Symbol *symbol, bool &isConstant)
	{
		isConstant = false;
		int ret = 0;

		// Check each definition that reaches this entry
		const IR::EntrySet &set = mUseDefs.defines(entry, symbol);
		for(IR::EntrySet::const_iterator it = set.begin(); it != set.end(); it++) {
			IR::Entry *def = *it;
			if(def->type != IR::Entry::TypeLoadImm) {
				// This definition is not constant, therefore the symbol can't be
				ret = 0;
				isConstant = false;
				break;
			}

			IR::EntryTwoAddrImm *twoAddrImm = (IR::EntryTwoAddrImm*)def;
			if(isConstant && ret != twoAddrImm->imm) {
				// If a constant was already found, and this one differs in value, then
				// the symbol is not constant
				ret = 0;
				isConstant = false;
				break;
			} else {
				// Otherwise, this is the first constant, so mark it as such
				ret = twoAddrImm->imm;
				isConstant = true;
			}
		}

		return ret;
	}
}