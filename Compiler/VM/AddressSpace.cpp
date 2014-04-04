#include "VM/AddressSpace.h"

namespace VM {

AddressSpace::AddressSpace()
{
}

AddressSpace::~AddressSpace()
{
	for(unsigned int i=0; i<mRegions.size(); i++) {
		delete[] mRegions[i].data;
	}
}

void AddressSpace::addRegion(unsigned int start, unsigned int size)
{
	Region region;
	region.start = start;
	region.size = size;
	region.data = new unsigned char[size];

	mRegions.push_back(region);
}

unsigned char *AddressSpace::at(unsigned int address)
{
	for(unsigned int i=0; i<mRegions.size(); i++) {
		if(address >= mRegions[i].start && address < mRegions[i].start + mRegions[i].size) {
			return mRegions[i].data + (address - mRegions[i].start);
		}
	}

	return 0;
}

}