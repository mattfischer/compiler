#ifndef FRONT_TOKENIZER_H
#define FRONT_TOKENIZER_H

#include <string>
#include <fstream>

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
class Tokenizer {
public:
	Tokenizer(const std::string &filename);

	/*!
	 * \brief Structure that represents a single token
	 */
	struct Token {
		/*!
		 * \brief Token type
		 */
		enum Type {
			TypeLiteral, //!< Literal text such as "+" or "while"
			TypeIdentifier, //!< An arbitrary text string
			TypeNumber, //!< A string of digits
			TypeEnd //!< Special token representing the end of the file
		};

		Type type; //!< Token type
		std::string text; //!< Textual content of token
		int line; //!< Starting line of token
		int column; //!< Starting column of token

		static std::string typeName(Type type);
	};

	const Token &next() { return mNext; } //!< Next token in token stream
	bool error() { return mError; } //!< True if an error has occurred
	const std::string &errorMessage() { return mErrorMessage; } //!< Error message, if any

	int line() { return mLine; } //!< Current line in token stream
	int column() { return mColumn; } //!< Current column in token stream

	void consume();

private:
	std::ifstream mStream; //!< Underlying file stream

	Token mNext; //!< Next token
	std::string mBuffer; //!< Buffer containing upcoming characters
	int mLine; //!< Current line
	int mColumn; //!< Current column

	bool mError; //!< True if error
	std::string mErrorMessage; //!< Error message, if any

	void getNext();
	bool fillBuffer(size_t length);
	void emptyBuffer(size_t length);
	void skipWhitespace();
};
}
#endif
