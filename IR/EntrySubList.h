#ifndef IR_ENTRY_SUB_LIST_H
#define IR_ENTRY_SUB_LIST_H

#include "IR/EntryList.h"

namespace IR {
	/*!
	 * \brief A subset of an entry list
	 *
	 * This class is an efficient representation of a subset of an EntryList class.  It
	 * operates by saving a start and end pointer into the EntryList's entries, so none
	 * of the entries are actually duplicated.  It provides iterators to iterate across
	 * the list just like a full EntryList does.
	 */
	class EntrySubList {
	public:
		typedef EntryList::const_iterator const_iterator;
		typedef EntryList::const_reverse_iterator const_reverse_iterator;

		class reverse_list {
		public:
			reverse_list(const EntrySubList &list) : mList(list) {}

			EntrySubList::const_reverse_iterator begin() const { return mList.rbegin(); }
			EntrySubList::const_reverse_iterator end() const { return mList.rend(); }

		private:
			const EntrySubList &mList;
		};

		EntrySubList(const_iterator begin, const_iterator end) : mBegin(begin), mEnd(end) {}
		EntrySubList() {}

		const_iterator begin() const { return mBegin; }
		const_iterator end() const { return mEnd; }
		const_reverse_iterator rbegin() const { return const_reverse_iterator(*--end()); }
		const_reverse_iterator rend() const { return const_reverse_iterator(*--begin()); }

		const Entry *front() const { return *begin(); }
		const Entry *back() const { return *(--end()); }

		reverse_list reversed() const { return reverse_list(*this); }

	private:
		const_iterator mBegin;
		const_iterator mEnd;
	};
}
#endif