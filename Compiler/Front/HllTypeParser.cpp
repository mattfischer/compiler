#include "Front/HllTypeParser.h"

namespace Front {

HllTypeParser::HllTypeParser(HllTokenizer &tokenizer)
 : Input::Parser(tokenizer)
{
}

/*!
 * \brief Parse type names from input
 * \return Set of type names declared in program
 */
std::set<std::string> HllTypeParser::parseTypes()
{
	std::set<std::string> types;
	types.insert("LibFoo");
	while(!match(HllTokenizer::TypeEnd)) {
		if(matchLiteral("class") || matchLiteral("struct")) {
			consume();
			types.insert(next().text);
		}
		consume();
	}

	return types;
}

}