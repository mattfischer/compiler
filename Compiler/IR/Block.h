#ifndef IR_BLOCK_H
#define IR_BLOCK_H

#include "IR/Entry.h"
#include "IR/EntryList.h"

#include <set>

namespace IR {
	class Block {
	public:
		int number;

		EntryList entries;

		Block(int _number) : number(_number)
		{
		}
	};
}
#endif
