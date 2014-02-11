#include "VM/Heap.h"

namespace VM {

	Heap::Heap(unsigned char *mem, int memSize, int start)
	{
		mMem = mem;
		mMemSize = memSize;
		mStart = start;

		mUsedSize = 0;
	}

	int Heap::allocate(int size)
	{
		int index = mStart + mUsedSize;
		mUsedSize += size;

		return index;
	}
}