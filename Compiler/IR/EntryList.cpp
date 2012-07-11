#include "IR/EntryList.h"

namespace IR {
	EntryList::EntryList() {
		mHead = NULL;
		mTail = NULL;
	}

	EntryList::iterator EntryList::insert(Entry *pos, Entry* entry)
	{
		if(pos) {
			entry->prev = pos->prev;
			if(pos->prev) {
				pos->prev->next = entry;
			}
			entry->next = pos;
			pos->prev = entry;
		} else {
			entry->next = 0;
			entry->prev = mTail;
			if(mTail) {
				mTail->next = entry;
			}
			mTail = entry;
		}

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

		if(entry == mTail) {
			mTail = mTail->prev;
		}

		return iterator(entry->next);
	}
}