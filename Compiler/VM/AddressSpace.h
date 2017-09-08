#ifndef VM_ADDRESS_SPACE_H
#define VM_ADDRESS_SPACE_H

#include <vector>
#include <memory>

namespace VM {
class AddressSpace {
public:
	void addRegion(unsigned int start, unsigned int size);

	unsigned char *at(unsigned int address);

private:
	struct Region {
		unsigned int start;
		std::vector<unsigned char> data;
	};

	std::vector<std::unique_ptr<Region>> mRegions;
};
}

#endif