#ifndef IR_BLOCK_H
#define IR_BLOCK_H

#include "IR/Entry.h"

#include <vector>

namespace IR {
	class Block {
	public:
		int number;
		std::vector<Block*> pred;
		std::vector<Block*> succ;

		Block(int _number) : number(_number), headEntry(Entry::TypeNone), tailEntry(Entry::TypeNone) 
		{
			headEntry.next = &tailEntry;
			tailEntry.prev = &headEntry;
		}

		void addPred(Block *block);
		void removePred(Block *block);
		void replacePred(Block *block, Block *newBlock);

		void addSucc(Block *block);
		void removeSucc(Block *block);
		void replaceSucc(Block *block, Block *newBlock);

		void appendEntry(Entry *entry) { entry->insertBefore(tail());}
		void prependEntry(Entry *entry) { entry->insertAfter(head());}

		Entry *head() { return &headEntry; }
		Entry *tail() { return &tailEntry; }

		Entry headEntry;
		Entry tailEntry;
	};
}
#endif
