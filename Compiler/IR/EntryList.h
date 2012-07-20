#ifndef IR_ENTRY_LIST_H
#define IR_ENTRY_LIST_H

#include "IR/Entry.h"

namespace IR {
	class EntryList {
	public:
		class iterator {
			friend class EntryList;
			friend class EntrySubList;

		public:
			iterator() : mEntry(0) {}
			Entry *operator*() { return mEntry; }
			iterator operator++(int) { iterator ret(mEntry); mEntry = mEntry->next; return ret;}
			iterator &operator++() { mEntry = mEntry->next; return *this; }
			iterator operator--(int) { iterator ret(mEntry); mEntry = mEntry->prev; return ret;}
			iterator &operator--() { mEntry = mEntry->prev; return *this; }
			bool operator==(const iterator &other) const { return mEntry == other.mEntry; }
			bool operator!=(const iterator &other) const { return mEntry != other.mEntry; }

		private:
			iterator(Entry *entry) : mEntry(entry) {}
			Entry *mEntry;
		};

		class reverse_iterator {
			friend class EntryList;
			friend class EntrySubList;

		public:
			reverse_iterator() : mEntry(0) {}
			Entry *operator*() { return mEntry; }
			reverse_iterator operator++(int) { reverse_iterator ret(mEntry); mEntry = mEntry->prev; return ret;}
			reverse_iterator &operator++() { mEntry = mEntry->prev; return *this; }
			reverse_iterator operator--(int) { reverse_iterator ret(mEntry); mEntry = mEntry->next; return ret;}
			reverse_iterator &operator--() { mEntry = mEntry->next; return *this; }
			bool operator==(const reverse_iterator &other) const { return mEntry == other.mEntry; }
			bool operator!=(const reverse_iterator &other) const { return mEntry != other.mEntry; }

		private:
			reverse_iterator(Entry *entry) : mEntry(entry) {}
			Entry *mEntry;
		};

		EntryList();

		iterator begin() { return iterator(mHead.next); }
		iterator end() { return iterator(&mTail); }
		reverse_iterator rbegin() { return reverse_iterator(mTail.prev); }
		reverse_iterator rend() { return reverse_iterator(&mHead); }

		Entry *front() { return *begin(); }
		Entry *back() { return *(--end()); }

		iterator push_back(Entry* entry) { return insert(end(), entry); }
		iterator push_front(Entry* entry) { return insert(begin(), entry); }
		iterator insert(Entry *position, Entry *entry);
		iterator insert(iterator position, Entry *entry) { return insert(*position, entry); }
		iterator find(Entry *entry) { return iterator(entry); }
		iterator erase(iterator position) { return erase(*position); }
		iterator erase(Entry *entry);
	private:
		Entry mHead;
		Entry mTail;
	};
}
#endif
