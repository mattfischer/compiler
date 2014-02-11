#ifndef VM_HEAP_H
#define VM_HEAP_H

namespace VM {
	class Heap {
	public:
		Heap(unsigned char *mem, int memSize, int start);

		int allocate(int size);

	private:
		unsigned char *mMem;
		int mMemSize;
		int mStart;

		int mUsedSize;
	};
}

#endif