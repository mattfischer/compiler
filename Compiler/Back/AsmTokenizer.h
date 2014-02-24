#ifndef BACK_ASM_TOKENIZER_H
#define BACK_ASM_TOKENIZER_H

#include "Input/Tokenizer.h"

#include <string>
#include <vector>

namespace Back {
/*!
 * \brief Tokenizer for assembly language
 *
 * A simple hand-written tokenizer that produces a token stream from an input file.
 */
class AsmTokenizer : public Input::Tokenizer {
public:
	AsmTokenizer(std::istream &stream);

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
