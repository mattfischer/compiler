#include "IR/Symbol.h"

namespace IR {
	void Symbol::addAssign(Entry *entry)
	{
		assigns.push_back(entry);
	}

	void Symbol::removeAssign(Entry *entry)
	{
		for(unsigned int i=0; i<assigns.size(); i++)
			if(assigns[i] == entry) {
				assigns.erase(assigns.begin() + i);
				break;
			}
	}
}