#include "Analysis/Constants.h"

namespace Analysis {
	Constants::Constants(const IR::Procedure &procedure, const UseDefs &useDefs)
		: mUseDefs(useDefs)
	{
	}

	/*!
	 * \brief Get the value of a symbol at an entry, if it can be determined to be a constant
	 * \param entry Entry to examine
	 * \param symbol Symbol to evaluate
	 * \param isConstant [out] True if symbol has a constant value, false otherwise
	 * \return Value of symbol
	 */
	int Constants::getIntValue(const IR::Entry *entry, const IR::Symbol *symbol, bool &isConstant) const
	{
		isConstant = false;
		int ret = 0;

		// Check each definition that reaches this entry
		const std::set<const IR::Entry*> &set = mUseDefs.defines(entry, symbol);
		for(const IR::Entry *def : set) {
			IR::EntryThreeAddr *threeAddr = (IR::EntryThreeAddr*)def;

			if(def->type != IR::Entry::Type::Move || threeAddr->rhs1) {
				// This definition is not constant, therefore the symbol can't be
				ret = 0;
				isConstant = false;
				break;
			}

			if(isConstant && ret != threeAddr->imm) {
				// If a constant was already found, and this one differs in value, then
				// the symbol is not constant
				ret = 0;
				isConstant = false;
				break;
			} else {
				// Otherwise, this is the first constant, so mark it as such
				ret = threeAddr->imm;
				isConstant = true;
			}
		}

		return ret;
	}

	/*!
	 * \brief Get the value of a symbol at an entry, if it can be determined to be a constant
	 * \param entry Entry to examine
	 * \param symbol Symbol to evaluate
	 * \param isConstant [out] True if symbol has a constant value, false otherwise
	 * \return Value of symbol
	 */
	std::string Constants::getStringValue(const IR::Entry *entry, const IR::Symbol *symbol, bool &isConstant) const
	{
		isConstant = false;
		std::string ret;

		// Check each definition that reaches this entry
		const std::set<const IR::Entry*> &set = mUseDefs.defines(entry, symbol);
		for(const IR::Entry *def : set) {
			IR::EntryString *string = (IR::EntryString*)def;

			if(def->type != IR::Entry::Type::LoadString) {
				// This definition is not constant, therefore the symbol can't be
				ret = "";
				isConstant = false;
				break;
			}

			if(isConstant && ret != string->string) {
				// If a constant was already found, and this one differs in value, then
				// the symbol is not constant
				ret = "";
				isConstant = false;
				break;
			} else {
				// Otherwise, this is the first constant, so mark it as such
				ret = string->string;
				isConstant = true;
			}
		}

		return ret;
	}
}