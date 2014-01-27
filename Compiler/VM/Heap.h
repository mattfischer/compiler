#ifndef VM_HEAP_H
#define VM_HEAP_H

namespace VM {
	class Heap {
	public:
		Heap(int *mem, int memSize, int start);

		int allocate(int size);

	private:
		int *mMem;
		int mMemSize;
		int mStart;

		int mUsedSize;
	};
}

#endif