#ifndef IR_ENTRY_SUB_LIST_H
#define IR_ENTRY_SUB_LIST_H

#include "IR/EntryList.h"

namespace IR {
	class EntrySubList {
	public:
		typedef EntryList::iterator iterator;
		typedef EntryList::reverse_iterator reverse_iterator;

		EntrySubList(iterator begin, iterator end) : mBegin(begin), mEnd(end) {}
		EntrySubList() {}

		iterator begin() { return mBegin; }
		iterator end() { return mEnd; }
		reverse_iterator rbegin() { return reverse_iterator(*--end()); }
		reverse_iterator rend() { return reverse_iterator(*begin()); }

		Entry *front() { return *begin(); }
		Entry *back() { return *(--end()); }

	private:
		iterator mBegin;
		iterator mEnd;
	};
};
#endif