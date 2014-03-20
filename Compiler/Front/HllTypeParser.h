#ifndef FRONT_HLL_TYPE_PARSER_H
#define FRONT_HLL_TYPE_PARSER_H

#include "Input/Parser.h"
#include "Front/HllTokenizer.h"

#include <set>
#include <string>

namespace Front {

/*!
 * \brief Small parser to extract type names from input file
 */
class HllTypeParser : public Input::Parser {
public:
	HllTypeParser(HllTokenizer &tokenizer);

	std::set<std::string> parseTypes();

private:
};

}

#endif