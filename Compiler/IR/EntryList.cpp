#include "IR/EntryList.h"

namespace IR {
	EntryList::EntryList() : mHead(IR::Entry::Type::None), mTail(IR::Entry::Type::None) {
		mHead.next = &mTail;
		mHead.prev = 0;
		mTail.next = 0;
		mTail.prev = &mHead;
	}

	EntryList::iterator EntryList::insert(Entry *pos, Entry* entry)
	{
		entry->prev = pos->prev;
		pos->prev->next = entry;
		entry->next = pos;
		pos->prev = entry;

		return iterator(entry);
	}

	EntryList::iterator EntryList::erase(Entry *entry)
	{
		entry->next->prev = entry->prev;
		entry->prev->next = entry->next;

		return iterator(entry->next);
	}
}