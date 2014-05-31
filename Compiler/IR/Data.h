#ifndef IR_DATA_H
#define IR_DATA_H

#include "IR/EntryList.h"

#include <string>
#include <list>

namespace IR {
	/*!
	 * \brief A data section in an IR program
	 */
	class Data {
	public:
		Data(const std::string &name);

		void print(std::ostream &o, const std::string &prefix = "");

		const std::string &name() const { return mName; } //!< Data section name
		EntryList &entries() { return mEntries; } //!< Body of data section

		void emit(Entry *entry);

	private:
		std::string mName; //!< Data section name
		EntryList mEntries; //!< Entry list
	};

	typedef std::list<Data*> DataList;
}

#endif
