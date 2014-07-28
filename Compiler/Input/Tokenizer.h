#ifndef INPUT_TOKENIZER_H
#define INPUT_TOKENIZER_H

#include <string>
#include <istream>
#include <vector>

namespace Input {
class Tokenizer {
public:
	Tokenizer(std::istream &stream, int lookahead);

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

	virtual std::string typeName(int type) = 0;

protected:
	const std::string &buffer() { return mBuffer; }
	bool fillBuffer(size_t length);
	void emptyBuffer(size_t length);
	void skipCharacters(const char characters[], int numCharacters);
	void setError(const std::string &message);

	Token createToken(int type, const std::string& text);

	bool scanLiteral(char *literals[], int numLiterals, Token &token);

	virtual Token getNext() = 0;

private:
	std::istream &mStream; //!< Underlying file stream
	std::string mBuffer; //!< Buffer containing upcoming characters
	int mLine; //!< Current line
	int mColumn; //!< Current column
	bool mPopulated; //!< True if lookahead has been populated
	std::vector<Token> mNext; //!< Next tokens

	bool mError; //!< True if error
	std::string mErrorMessage; //!< Error message, if any
};
}

#endif