#include "Front/Program.h"

namespace Front {
	void Program::print()
	{
		for(unsigned int i=0; i<procedures.size(); i++) {
			procedures[i]->print();
			std::cout << std::endl;
		}
	}
}