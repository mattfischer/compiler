#ifndef FRONT_TOKENIZER_H
#define FRONT_TOKENIZER_H

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

class BaseTokenizer {
public:
	BaseTokenizer(const std::string &filename, int lookahead);

	/*!
	 * \brief Structure that represents a single token
	 */
	struct Token {
		enum Type {
			TypeEnd,
			TypeLiteral
		};

		Type type; //!< Token type
		std::string text; //!< Textual content of token
		int line; //!< Starting line of token
		int column; //!< Starting column of token
	};

	bool error() { return mError; } //!< True if an error has occurred
	const std::string &errorMessage() { return mErrorMessage; } //!< Error message, if any

	int line() { return mLine; } //!< Current line in token stream
	int column() { return mColumn; } //!< Current column in token stream

	const Token &next(int num = 0);
	void consume();

protected:
	const std::string &buffer() { return mBuffer; }
	bool fillBuffer(size_t length);
	void emptyBuffer(size_t length);
	void skipWhitespace();
	void setError(const std::string &message);

	Token createToken(int type, const std::string& text);

	bool scanLiteral(char *literals[], int numLiterals, Token &token);

	virtual Token getNext() = 0;

private:
	std::ifstream mStream; //!< Underlying file stream
	std::string mBuffer; //!< Buffer containing upcoming characters
	int mLine; //!< Current line
	int mColumn; //!< Current column
	bool mPopulated; //!< True if lookahead has been populated
	std::vector<Token> mNext; //!< Next tokens

	bool mError; //!< True if error
	std::string mErrorMessage; //!< Error message, if any
};

/*!
 * \brief Tokenizer for source language
 *
 * A simple hand-written tokenizer that produces a token stream from an input file.
 */
class Tokenizer : public BaseTokenizer {
public:
	Tokenizer(const std::string &filename);

	/*!
	 * \brief Token type
	 */
	enum TokenType {
		TypeEnd = BaseTokenizer::Token::TypeEnd, //!< Special token representing the end of the file
		TypeLiteral = BaseTokenizer::Token::TypeLiteral, //!< Literal text such as "+" or "while"
		TypeIdentifier, //!< An arbitrary text string
		TypeNumber, //!< A string of digits
		TypeString //!< A string literal
	};

	static std::string typeName(TokenType type);

private:
	virtual Token getNext();
};
}
#endif
