#include "Transform/CopyProp.h"

#include "IR/Procedure.h"
#include "IR/Entry.h"
#include "IR/Symbol.h"
#include "Analysis/Analysis.h"
#include "Analysis/UseDefs.h"
#include "Analysis/ReachingDefs.h"
#include "Analysis/DataFlow.h"

namespace Transform {
	bool CopyProp::transform(IR::Procedure *procedure, Analysis::Analysis &analysis)
	{
		bool changed = false;
		IR::EntrySet allLoads;
		std::map<IR::Entry*, IR::EntrySet> gen;
		std::map<IR::Entry*, IR::EntrySet> kill;

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			if(entry->type == IR::Entry::TypeLoad) {
				allLoads.insert(entry);
				gen[entry].insert(entry);
			}
		}

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			if(!entry->assign()) {
				continue;
			}

			for(IR::EntrySet::iterator itLoad = allLoads.begin(); itLoad != allLoads.end(); itLoad++) {
				IR::Entry *load = *itLoad;
				if(entry != load && (load->assign() == entry->assign() || load->uses(entry->assign()))) {
					kill[entry].insert(load);
				}
			}
		}

		Analysis::DataFlow<IR::Entry*> dataFlow;
		std::map<IR::Entry*, IR::EntrySet> loads = dataFlow.analyze(analysis.flowGraph(), gen, kill, allLoads, Analysis::DataFlow<IR::Entry*>::MeetTypeIntersect, Analysis::DataFlow<IR::Entry*>::DirectionForward);

		for(IR::EntryList::iterator itEntry = procedure->entries().begin(); itEntry != procedure->entries().end(); itEntry++) {
			IR::Entry *entry = *itEntry;

			IR::EntrySet &c = loads[entry];
			for(IR::EntrySet::iterator itLoad = c.begin(); itLoad != c.end(); itLoad++) {
				IR::EntryThreeAddr *load = (IR::EntryThreeAddr*)*itLoad;
				if(entry->uses(load->lhs)) {
					entry->replaceUse(load->lhs, load->rhs1);
					analysis.replaceUse(entry, load->lhs, load->rhs1);
					changed = true;
				}
			}
		}

		return changed;
	}

	CopyProp *CopyProp::instance()
	{
		static CopyProp inst;
		return &inst;
	}
}