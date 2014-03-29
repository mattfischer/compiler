#ifndef VM_HEAP_H
#define VM_HEAP_H

namespace VM {
	class Heap {
	public:
		Heap(unsigned int start);
		~Heap();

		unsigned int allocate(unsigned int size);
		void free(unsigned int index);
		unsigned char *at(unsigned int index);

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
		unsigned int mSize;
		unsigned int mUsedSize;
		Header *mFreeList;
	};
}

#endif