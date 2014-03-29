#include "VM/Heap.h"

namespace VM {

	const int WordSize = sizeof(unsigned long);
	const int MinSize = 2 * WordSize;
	const int PrevInUseBit = 0x1;

	Heap::Heap(unsigned char *mem, int memSize, int start)
	{
		mMem = mem;
		mMemSize = memSize;
		mStart = start;

		mUsedSize = 0;
		mFreeList = 0;
	}

	unsigned int Heap::allocate(unsigned int size)
	{
		if(size % WordSize != 0) {
			size += WordSize - size % WordSize;
		}

		if(size < MinSize) {
			size = MinSize;
		}

		size += WordSize;

		Header *header = 0;
		Header *cursor = mFreeList;
		while(cursor) {
			if(cursor->size >= size) {
				header = cursor;
				removeFree(header);

				Header *nextHeader = getHeader(getIndex(header) + header->size);
				nextHeader->size |= PrevInUseBit;
				break;
			}
			cursor = cursor->nextFree;
		}

		if(!header) {
			int index = mStart + mUsedSize + WordSize;
			mUsedSize += size;

			header = getHeader(index);
			header->size = size;
			header->size |= PrevInUseBit;
		}

		return getIndex(header);
	}

	void Heap::free(unsigned int index)
	{
		Header *header = getHeader(index);
		Header *nextHeader = getHeader(index + header->size);

		nextHeader->size &= ~PrevInUseBit;
		nextHeader->prevSize = header->size;

		if(index == mStart + mUsedSize - header->size) {
			mUsedSize -= header->size;
		} else {
			pushFree(header);
		}
	}

	Heap::Header *Heap::getHeader(unsigned int index)
	{
		return (Header*)(mMem + index - 2 * WordSize);
	}

	unsigned long Heap::getIndex(Header *header)
	{
		return (unsigned long)((unsigned char*)header - (unsigned char*)mMem) + 2 * WordSize;
	}

	void Heap::pushFree(Header *header)
	{
		if(mFreeList) {
			mFreeList->prevFree = header;
		}
		header->nextFree = mFreeList;
		header->prevFree = 0;
		mFreeList = header;
	}

	void Heap::removeFree(Header *header)
	{
		if(header->prevFree) {
			header->prevFree->nextFree = header->nextFree;
		}

		if(header->nextFree) {
			header->nextFree->prevFree = header->prevFree;
		}

		if(header == mFreeList) {
			mFreeList = header->nextFree;
		}
	}
}