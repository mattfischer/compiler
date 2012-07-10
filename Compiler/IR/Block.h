#ifndef IR_BLOCK_H
#define IR_BLOCK_H

#include "IR/Entry.h"

#include <set>

namespace IR {
	class Block {
	public:
		int number;
		typedef std::set<Block*> BlockSet;

		Block(int _number) : number(_number), headEntry(Entry::TypeNone), tailEntry(Entry::TypeNone) 
		{
			headEntry.next = &tailEntry;
			tailEntry.prev = &headEntry;
		}

		void appendEntry(Entry *entry) { entry->insertBefore(tail());}
		void prependEntry(Entry *entry) { entry->insertAfter(head());}

		Entry *head() { return &headEntry; }
		Entry *tail() { return &tailEntry; }

		Entry headEntry;
		Entry tailEntry;
	};
}
#endif
