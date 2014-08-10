#include "VM/AddressSpace.h"

namespace VM {

AddressSpace::AddressSpace()
{
}

AddressSpace::~AddressSpace()
{
	for(Region &region : mRegions) {
		delete[] region.data;
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
	for(Region &region : mRegions) {
		if(address >= region.start && address < region.start + region.size) {
			return region.data + (address - region.start);
		}
	}

	return 0;
}

}