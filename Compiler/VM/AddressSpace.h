#ifndef VM_ADDRESS_SPACE_H
#define VM_ADDRESS_SPACE_H

#include <vector>

namespace VM {
class AddressSpace {
public:
	AddressSpace();
	~AddressSpace();

	void addRegion(unsigned int start, unsigned int size);

	unsigned char *at(unsigned int address);

private:
	struct Region {
		unsigned int start;
		unsigned int size;
		unsigned char *data;
	};

	std::vector<Region> mRegions;
};
}

#endif