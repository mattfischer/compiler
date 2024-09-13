#include "Front/Program.h"

namespace Front {
	void Program::print(std::ostream &o)
	{
		for(std::unique_ptr<Procedure> &procedure : procedures) {
			procedure->print(o);
			o << std::endl;
		}
	}
}