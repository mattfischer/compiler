#ifndef VM_GARBAGE_COLLECTOR_H
#define VM_GARBAGE_COLLECTOR_H

#include "VM/Heap.h"

namespace VM {

class GarbageCollector {
public:
	GarbageCollector(Heap &heap);

	void collect(int *regs, unsigned int stackTop);

private:
	void markAllocation(unsigned int index);

	Heap &mHeap;
};

}

#endif