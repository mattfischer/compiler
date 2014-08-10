#include "Front/Program.h"

namespace Front {
	void Program::print(std::ostream &o)
	{
		for(Procedure *procedure : procedures) {
			procedure->print(o);
			o << std::endl;
		}
	}
}