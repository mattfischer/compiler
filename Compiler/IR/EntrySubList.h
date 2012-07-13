#ifndef IR_ENTRY_SUB_LIST_H
#define IR_ENTRY_SUB_LIST_H

#include "IR/EntryList.h"

namespace IR {
	class EntrySubList {
	public:
		typedef EntryList::iterator iterator;

		EntrySubList(iterator begin, iterator end) : mBegin(begin), mEnd(end) {}
		EntrySubList() {}

		iterator begin() { return mBegin; }
		iterator end() { return mEnd; }
		Entry *front() { return *begin(); }
		Entry *back() { return *(--end()); }

	private:
		iterator mBegin;
		iterator mEnd;
	};
};
#endif