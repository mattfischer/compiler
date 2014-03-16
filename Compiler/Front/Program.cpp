#include "Front/Program.h"

namespace Front {
	void Program::print(std::ostream &o)
	{
		for(unsigned int i=0; i<procedures.size(); i++) {
			procedures[i]->print(o);
			o << std::endl;
		}
	}
}