#ifndef VM_HEAP_H
#define VM_HEAP_H

namespace VM {
	class Heap {
	public:
		Heap(unsigned char *mem, int memSize, int start);

		unsigned int allocate(unsigned int size);
		void free(unsigned int index);

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

		unsigned char *mMem;
		int mMemSize;
		int mStart;

		int mUsedSize;
		Header *mFreeList;
	};
}

#endif