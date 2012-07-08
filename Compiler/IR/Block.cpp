#include "IR/Block.h"

namespace IR {
	void Block::addPred(Block *block)
	{
		bool found = false;

		for(unsigned int i=0; i<pred.size(); i++) {
			if(pred[i] == block) {
				found = true;
				break;
			}
		}

		if(!found)
			pred.push_back(block);
	}

	void Block::removePred(Block *block)
	{
		for(unsigned int i=0; i<pred.size(); i++) {
			if(pred[i] == block) {
				pred.erase(pred.begin() + i);
				for(Entry *entry = head()->next; entry != tail(); entry = entry->next) {
					if(entry->type == Entry::TypePhi) {
						EntryPhi *phi = (EntryPhi*)entry;
						phi->removeArg(i);
					}
				}
				break;
			}
		}
	}

	void Block::replacePred(Block *block, Block *newBlock)
	{
		for(unsigned int i=0; i<pred.size(); i++) {
			if(pred[i] == block) {
				pred[i] = newBlock;
				break;
			}
		}
	}

	void Block::addSucc(Block *block)
	{
		bool found = false;

		for(unsigned int i=0; i<succ.size(); i++) {
			if(succ[i] == block) {
				found = true;
				break;
			}
		}

		if(!found)
			succ.push_back(block);
	}

	void Block::removeSucc(Block *block)
	{
		for(unsigned int i=0; i<succ.size(); i++) {
			if(succ[i] == block) {
				succ.erase(succ.begin() + i);
				break;
			}
		}
	}

	void Block::replaceSucc(Block *block, Block *newBlock)
	{
		for(unsigned int i=0; i<succ.size(); i++) {
			if(succ[i] == block) {
				succ[i] = newBlock;
				break;
			}
		}
	}
}