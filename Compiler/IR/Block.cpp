#include "IR/Block.h"

#include <sstream>

namespace IR {
	Block::Block(int _number)
	 : number(_number)
	{
		std::stringstream ss;
		ss << number;
		label = new IR::EntryLabel("bb" + ss.str(), this);
		entries.push_back(label);
	}
}