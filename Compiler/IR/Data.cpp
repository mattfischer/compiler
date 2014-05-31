#include "IR/Data.h"

namespace IR {
	Data::Data(const std::string &name)
	{
		mName = name;
	}

	/*!
	 * \brief Print procedure
	 * \param prefix Prefix to put before each line of output
	 */
	void Data::print(std::ostream &o, const std::string &prefix)
	{
		for(IR::EntryList::iterator itEntry = mEntries.begin(); itEntry != mEntries.end(); itEntry++) {
			IR::Entry *entry = *itEntry;
			o << prefix << *entry << std::endl;
		}
		o << prefix << std::endl;
	}

	/*!
	 * \brief Add an entry to the end of the section
	 * \param entry Entry to add
	 */
	void Data::emit(Entry *entry)
	{
		mEntries.insert(mEntries.end(), entry);
	}
}
