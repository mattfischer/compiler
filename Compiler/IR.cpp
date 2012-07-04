#include "IR.h"

void IR::Block::addPred(Block *block)
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

void IR::Block::removePred(Block *block)
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

void IR::Block::replacePred(Block *block, Block *newBlock)
{
	for(unsigned int i=0; i<pred.size(); i++) {
		if(pred[i] == block) {
			pred[i] = newBlock;
			break;
		}
	}
}

void IR::Block::addSucc(Block *block)
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

void IR::Block::removeSucc(Block *block)
{
	for(unsigned int i=0; i<succ.size(); i++) {
		if(succ[i] == block) {
			succ.erase(succ.begin() + i);
			break;
		}
	}
}

void IR::Block::replaceSucc(Block *block, Block *newBlock)
{
	for(unsigned int i=0; i<succ.size(); i++) {
		if(succ[i] == block) {
			succ[i] = newBlock;
			break;
		}
	}
}

void IR::Symbol::addAssign(Entry *entry)
{
	assigns.push_back(entry);
}

void IR::Symbol::removeAssign(Entry *entry)
{
	for(unsigned int i=0; i<assigns.size(); i++)
		if(assigns[i] == entry) {
			assigns.erase(assigns.begin() + i);
			break;
		}
}

IR::IR()
{
	mMain = new Procedure("main");
	mProcedures.push_back(mMain);
}

void IR::print() const
{
	for(unsigned int i=0; i<mProcedures.size(); i++) {
		printf("<%s>\n", mProcedures[i]->name().c_str());
		mProcedures[i]->print("  ");
	}
}
