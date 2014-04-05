#include "VM/Heap.h"

namespace VM {

	const int WordSize = sizeof(unsigned long);
	const int MinSize = 2 * WordSize;
	const int PrevInUseBit = 0x1;
	const int MarkBit = 0x2;
	const unsigned int SizeMask = 0xfffffffc;

	Heap::Heap(AddressSpace &addressSpace, unsigned int start, unsigned int size)
		: mAddressSpace(addressSpace)
	{
		mStart = start;
		mSize = size;
		mAddressSpace.addRegion(start, size);

		mUsedSize = 2 * WordSize;
		mFreeList = 0;
	}

	Heap::~Heap()
	{
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
			int index = mStart + mUsedSize;
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
		int size = header->size & SizeMask;
		Header *nextHeader = getHeader(index + size);

		nextHeader->size &= ~PrevInUseBit;
		nextHeader->prevSize = size;

		if(index == mUsedSize - size) {
			mUsedSize -= size;
		} else {
			pushFree(header);
		}
	}

	Heap::Header *Heap::getHeader(unsigned int index)
	{
		return (Header*)mAddressSpace.at(index - 2 * WordSize);
	}

	unsigned long Heap::getIndex(Header *header)
	{
		return (unsigned long)((unsigned char*)header - mAddressSpace.at(mStart)) + mStart + 2 * WordSize;
	}

	unsigned int Heap::firstAllocation()
	{
		if(mUsedSize > 2 * WordSize) {
			return mStart + 2 * WordSize;
		} else {
			return 0;
		}
	}

	unsigned int Heap::nextAllocation(unsigned int index)
	{
		Header *header = getHeader(index);
		index += header->size & SizeMask;
		if(index < mStart + mUsedSize) {
			return index;
		} else {
			return 0;
		}
	}

	unsigned int Heap::allocationSize(unsigned int index)
	{
		Header *header = getHeader(index);
		return (header->size & SizeMask) - WordSize;
	}

	bool Heap::allocationMarked(unsigned int index)
	{
		Header *header = getHeader(index);
		return (header->size & MarkBit) != 0;
	}

	void Heap::setAllocationMarked(unsigned int index, bool marked)
	{
		Header *header = getHeader(index);
		header->size &= ~MarkBit;
		if(marked) {
			header->size |= MarkBit;
		}
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