#include "Input/Parser.h"

#include <sstream>

namespace Input {
/*!
 * \brief Constructor.
 * \param tokenizer Input token source
 */
Parser::Parser(Tokenizer &tokenizer)
	: mTokenizer(tokenizer)
{
	mError = false;
}

/*!
 * \brief Get the next token in the stream
 * \param Token number
 * \return Next token
 */
const Tokenizer::Token &Parser::next(int num)
{
	return mTokenizer.next(num);
}

/*!
 * \brief Consume the next token if it matches the given type, throw an error if it doesn't
 * \param type Type of token to expect
 */
void Parser::expect(int type)
{
	// If the tokenizer has an error, propagate it up
	if(mTokenizer.error()) {
		throw ParseException(mTokenizer.errorMessage(), next().line, next().column);
	}

	// If the token type doesn't match throw an error
	if(next().type != type) {
		errorExpected(mTokenizer.typeName(type));
	}

	// Otherwise, consume it
	consume();
}

/*!
 * \brief Consume the next token if it is a literal with the given text, throw an error if it is not
 * \param text Text of expected literal
 */
void Parser::expectLiteral(const std::string &text)
{
	// If the tokenizer has an error, propagate it up
	if(mTokenizer.error()) {
		throw ParseException(mTokenizer.errorMessage(), next().line, next().column);
	}

	// If the next token is not a literal, or does not have the expected text, throw an error
	if(next().type != Input::Tokenizer::Token::TypeLiteral || next().text != text) {
		errorExpected(text);
	}

	// Otherwise, consume it
	consume();
}

/*!
 * \brief Check if the next token is of a given type
 * \param type Type of token to match
 * \param num Lookahead number
 * \return True if next token matches
 */
bool Parser::match(int type, int num)
{
	// Check if the tokenizer has an error or if the next token is of a different type
	if(mTokenizer.error() || next(num).type != type) {
		return false;
	}

	return true;
}

/*!
 * \brief Check if the next token is a literal with the given text
 * \param text Text of literal to match
 * \param num Lookahead number
 * \return True if next token matches
 */
bool Parser::matchLiteral(const std::string &text, int num)
{
	// Check if the tokenizer has an error or if the next token is a literal with the given text
	if(mTokenizer.error() || next(num).type != Input::Tokenizer::Token::TypeLiteral || next(num).text != text) {
		return false;
	}

	return true;
}

/*!
 * \brief Consume the next tokens
 * \param num Number of tokens to conume
 */
void Parser::consume(int num)
{
	for(int i=0; i<num; i++) {
		mTokenizer.consume();
	}
}

/*!
 * \brief Throw an error stating that expected token was not found.  Never returns.
 * \param expected Expected token/node that was not found in input stream
 */
void Parser::errorExpected(const std::string &expected)
{
	std::stringstream ss;
	ss << "expected " << expected << ", '" << next().text << "' found instead";
	throw ParseException(ss.str(), next().line, next().column);
}

/*!
 * \brief Report an error
 * \param message Error message
 * \param line Line number
 * \param column Column
 */
void Parser::setError(const std::string &message, int line, int column)
{
	mError = true;
	mErrorMessage = message;
	mErrorLine = line;
	mErrorColumn = column;
}

}