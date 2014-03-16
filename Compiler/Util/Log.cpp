#include "Util/Log.h"

namespace Util {

std::ostream &log(const std::string &name)
{
	return std::cout;
}

}