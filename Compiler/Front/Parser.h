#ifndef FRONT_PARSER_H
#define FRONT_PARSER_H

#include <string>

namespace IR {
	class Program;
}

namespace Front {
	class Parser {
	public:
		Parser(const std::string &filename);

		IR::Program *ir();

	private:
		IR::Program *mIR;
	};
}

#endif