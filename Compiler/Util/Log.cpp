#include "Util/Log.h"
#include "Util/Array.h"

#include <fstream>

namespace Util {

const char *enabledLogs[] = {
	"asm",
	"output"
};

std::ofstream nullstream;

std::ostream &log(const std::string &name)
{
	for(int i=0; i<N_ELEMENTS(enabledLogs); i++) {
		if(enabledLogs[i] == name) {
			return std::cout;
		}
	}

	return nullstream;
}

}