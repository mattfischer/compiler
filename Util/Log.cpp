#include "Util/Log.h"

#include <fstream>
#include <vector>

namespace Util {

std::vector<std::string> enabledLogs = {
	"asm",
	"output"
};

std::ofstream nullstream;

std::ostream &log(const std::string &name)
{
	for(std::string &log : enabledLogs) {
		if(log == name) {
			return std::cout;
		}
	}

	return nullstream;
}

}