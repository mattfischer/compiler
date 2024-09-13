#ifndef VM_HEAP_H
#define VM_HEAP_H

#include "VM/AddressSpace.h"

namespace VM {
	class Heap {
	public:
		Heap(AddressSpace &addressSpace, unsigned int start, unsigned int size);
		~Heap();

		unsigned int allocate(unsigned int size);
		void free(unsigned int index);

		unsigned int start() { return mStart; }
		unsigned int size() { return mSize; }
		AddressSpace &addressSpace() { return mAddressSpace; }

		unsigned int firstAllocation();
		unsigned int nextAllocation(unsigned int index);

		unsigned int allocationSize(unsigned int index);
		bool allocationMarked(unsigned int index);
		void setAllocationMarked(unsigned int index, bool marked);

	private:
		struct Header {
			unsigned long prevSize;
			unsigned long size;
			Header *prevFree;
			Header *nextFree;
		};

		Header *getHeader(unsigned int index);
		unsigned long getIndex(Header *header);
		void pushFree(Header *header);
		void removeFree(Header *header);

		AddressSpace &mAddressSpace;
		unsigned int mStart;
		unsigned int mSize;
		unsigned int mUsedSize;
		Header *mFreeList;
	};
}

#endif