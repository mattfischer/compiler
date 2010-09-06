#include "OptPassIntoSSA.h"

#include <queue>
#include <sstream>

static bool assigns(IR::Entry *entry, IR::Symbol *symbol)
{
	switch(entry->type) {
		case IR::Entry::TypeAdd:
		case IR::Entry::TypeMult:
		case IR::Entry::TypeEqual:
		case IR::Entry::TypeNequal:
		case IR::Entry::TypeLoad:
			if(((IR::Entry::ThreeAddr*)entry)->lhs == symbol)
				return true;
			break;

		case IR::Entry::TypeLoadImm:
			if(((IR::Entry::Imm*)entry)->lhs == symbol)
				return true;
			break;

		case IR::Entry::TypePhi:
			if(((IR::Entry::Phi*)entry)->lhs == symbol)
				return true;
			break;

		default:
			break;
	}

	return false;
}

static void rewriteAssign(IR::Entry *entry, IR::Symbol *symbol)
{
		switch(entry->type) {
		case IR::Entry::TypeAdd:
		case IR::Entry::TypeMult:
		case IR::Entry::TypeEqual:
		case IR::Entry::TypeNequal:
		case IR::Entry::TypeLoad:
			((IR::Entry::ThreeAddr*)entry)->lhs = symbol;
			break;

		case IR::Entry::TypeLoadImm:
			((IR::Entry::Imm*)entry)->lhs = symbol;
			break;

		case IR::Entry::TypePhi:
			((IR::Entry::Phi*)entry)->lhs = symbol;

		default:
			break;
	}
}

static bool uses(IR::Entry *entry, IR::Symbol *symbol)
{
	switch(entry->type) {
		case IR::Entry::TypeAdd:
		case IR::Entry::TypeMult:
		case IR::Entry::TypeEqual:
		case IR::Entry::TypeNequal:
		case IR::Entry::TypeLoad:
			if(((IR::Entry::ThreeAddr*)entry)->rhs1 == symbol ||
			   ((IR::Entry::ThreeAddr*)entry)->rhs2 == symbol)
				return true;
			break;

		case IR::Entry::TypeCJump:
			if(((IR::Entry::CJump*)entry)->pred == symbol)
				return true;
			break;

		default:
			break;
	}

	return false;
}

static void rewriteUse(IR::Entry *entry, IR::Symbol *symbol, IR::Symbol *newSymbol)
{
	switch(entry->type) {
		case IR::Entry::TypeAdd:
		case IR::Entry::TypeMult:
		case IR::Entry::TypeEqual:
		case IR::Entry::TypeNequal:
		case IR::Entry::TypeLoad:
			{
				IR::Entry::ThreeAddr *threeAddr = (IR::Entry::ThreeAddr*)entry;

				if(threeAddr->rhs1 == symbol)
					threeAddr->rhs1 = newSymbol;

				if(threeAddr->rhs2 == symbol)
					threeAddr->rhs2 = newSymbol;
			}
			break;

		case IR::Entry::TypeCJump:
			((IR::Entry::CJump*)entry)->pred = newSymbol;
			break;

		default:
			break;
	}
}

static std::string newSymbolName(IR::Symbol *base, int version)
{
	std::stringstream ss;

	ss << base->name << "_" << version;
	return ss.str();
}

void OptPassIntoSSA::optimizeProcedure(IR::Procedure *proc)
{
	proc->computeDominance();

	for(unsigned int i=0; i<proc->symbols().size(); i++) {
		IR::Symbol *symbol = proc->symbols()[i];
		std::queue<IR::Block*> blocks;

		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];

			for(unsigned int k=0; k<block->entries.size(); k++) {
				IR::Entry *entry = block->entries[k];

				if(assigns(entry, symbol)) {
					blocks.push(block);
				}
			}
		}

		while(!blocks.empty()) {
			IR::Block *block = blocks.front();
			blocks.pop();

			for(unsigned int j=0; j<block->domFrontiers.size(); j++) {
				IR::Block *frontier = block->domFrontiers[j];
				IR::Entry *head = frontier->head();
				
				if(!head || head->type != IR::Entry::TypePhi || ((IR::Entry::Phi*)head)->lhs != symbol) {
					frontier->prependEntry(IR::Entry::newPhi(symbol, symbol, frontier->pred.size()));
					blocks.push(frontier);
				}
			}
		}

		int nextVersion = 0;
		std::vector<IR::Symbol*> activeList(proc->blocks().size());
		activeList[0] = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->type);
		for(unsigned int j=0; j<proc->blocks().size(); j++) {
			IR::Block *block = proc->blocks()[j];
			if(!activeList[j])
				activeList[j] = activeList[block->idom->number];
			IR::Symbol *active = activeList[j];

			for(unsigned int k=0; k<block->entries.size(); k++) {
				IR::Entry *entry = block->entries[k];

				if(uses(entry, symbol)) {
					rewriteUse(entry, symbol, active);
				}

				if(assigns(entry, symbol)) {
					active = new IR::Symbol(newSymbolName(symbol, nextVersion++), symbol->type);
					activeList[j] = active;
					rewriteAssign(entry, active);
				}
			}

			for(unsigned int k=0; k<block->succ.size(); k++) {
				IR::Block *succ = block->succ[k];
				IR::Entry *head = succ->head();

				if(head && head->type == IR::Entry::TypePhi && ((IR::Entry::Phi*)head)->base == symbol) {
					for(unsigned int l=0; l<succ->pred.size(); l++) {
						if(succ->pred[l] == block) {
							((IR::Entry::Phi*)head)->args[l] = active;
							break;
						}
					}
				}
			}
		}
	}
}