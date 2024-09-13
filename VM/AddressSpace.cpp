#include "VM/AddressSpace.h"

namespace VM {

void AddressSpace::addRegion(unsigned int start, unsigned int size)
{
	std::unique_ptr<Region> region = std::make_unique<Region>();
	region->start = start;
	region->data.resize(size);

	mRegions.push_back(std::move(region));
}

unsigned char *AddressSpace::at(unsigned int address)
{
	for(std::unique_ptr<Region> &region : mRegions) {
		if(address >= region->start && address < region->start + region->data.size()) {
			return &region->data[0] + (address - region->start);
		}
	}

	return 0;
}

}