#include "IR/EntryList.h"

namespace IR {
	EntryList::EntryList() : mTail(IR::Entry::TypeNone) {
		mHead = &mTail;
		mTail.prev = 0;
		mTail.next = 0;
	}

	EntryList::iterator EntryList::insert(Entry *pos, Entry* entry)
	{
		entry->prev = pos->prev;
		if(pos->prev) {
			pos->prev->next = entry;
		}
		entry->next = pos;
		pos->prev = entry;

		if(pos == mHead) {
			mHead = entry;
		}

		return iterator(entry);
	}

	EntryList::iterator EntryList::erase(Entry *entry)
	{
		if(entry->next != 0) {
			entry->next->prev = entry->prev;
		}

		if(entry->prev != 0) {
			entry->prev->next = entry->next;
		}

		if(entry == mHead) {
			mHead = mHead->next;
		}

		return iterator(entry->next);
	}
}