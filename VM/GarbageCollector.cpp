#include "VM/GarbageCollector.h"

#include "VM/Interp.h"

namespace VM {

GarbageCollector::GarbageCollector(Heap &heap)
 : mHeap(heap)
{
}

void GarbageCollector::collect(int *regs, unsigned int stackTop)
{
	for(unsigned int i = mHeap.firstAllocation(); i != 0; i = mHeap.nextAllocation(i)) {
		mHeap.setAllocationMarked(i, false);
	}

	for(int i=0; i<16; i++) {
		markAllocation((unsigned int)regs[i]);
	}

	for(unsigned int i = regs[VM::RegSP]; i < stackTop; i += sizeof(unsigned int)) {
		unsigned int *p = (unsigned int *)mHeap.addressSpace().at(i);
		markAllocation(*p);
	}

	for(unsigned int i = mHeap.firstAllocation(); i != 0; i = mHeap.nextAllocation(i)) {
		if(!mHeap.allocationMarked(i)) {
			mHeap.free(i);
		}
	}

}

void GarbageCollector::markAllocation(unsigned int index)
{
	if(index >= mHeap.start() && index < mHeap.start() + mHeap.size()) {
		if(!mHeap.allocationMarked(index)) {
			mHeap.setAllocationMarked(index, true);
			for(unsigned int i=0; i<mHeap.allocationSize(index); i += sizeof(unsigned int)) {
				unsigned int *p = (unsigned int*)mHeap.addressSpace().at(index + i);
				markAllocation(*p);
			}
		}
	}
}

}