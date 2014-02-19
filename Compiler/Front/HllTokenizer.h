#ifndef FRONT_TOKENIZER_H
#define FRONT_TOKENIZER_H

#include "Input/Tokenizer.h"

#include <string>
#include <fstream>
#include <vector>

/*!
 * \brief Front-end functions for the compiler
 *
 * The Front namespace contains tokenizer, parser, type checker, and IR generator for
 * the compiler.
 */
namespace Front {
/*!
 * \brief Tokenizer for source language
 *
 * A simple hand-written tokenizer that produces a token stream from an input file.
 */
class HllTokenizer : public Input::Tokenizer {
public:
	HllTokenizer(const std::string &filename);

	/*!
	 * \brief Token type
	 */
	enum TokenType {
		TypeEnd = Input::Tokenizer::Token::TypeEnd, //!< Special token representing the end of the file
		TypeLiteral = Input::Tokenizer::Token::TypeLiteral, //!< Literal text such as "+" or "while"
		TypeIdentifier, //!< An arbitrary text string
		TypeNumber, //!< A string of digits
		TypeString //!< A string literal
	};

	std::string typeName(int type);

private:
	virtual Token getNext();
};
}
#endif
