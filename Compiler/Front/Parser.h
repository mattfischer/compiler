#ifndef FRONT_PARSER_H
#define FRONT_PARSER_H

#include <string>

namespace IR {
	class Program;
}

namespace Front {
	class Parser {
	public:
		static IR::Program *parse(const std::string &filename);
	};
}

#endif