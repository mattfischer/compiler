#include "Front/Tokenizer.h"

#include <cctype>
#include <sstream>

char whitespace[] = { ' ', '\t', '\r', '\n' };
char *literals[] = { "print", "if", "else", "while", "return", "new", "for", "break",
					 "continue", "true", "false", "struct",
					 "==", "!=", ">=", "<=", "++", "--", "&&", "||",
					 ">", "<", "+", "-", "*", "(", ")", "=", ";", "{", "}", ",", "[", "]", "."
					};

namespace Front {

/*!
 * \brief Constructor
 * \param filename Filename to read
 * \param lookahead Number of lookahead tokens
 */
BaseTokenizer::BaseTokenizer(const std::string &filename, int lookahead)
	: mStream(filename.c_str())
{
	mLine = 1;
	mColumn = 1;
	mError = false;
	mPopulated = false;
	mNext.resize(lookahead);
}

/*!
 * \brief Return the next token in the stream
 * \param num Lookahead number
 * \return Token in stream
 */
const BaseTokenizer::Token &BaseTokenizer::next(int num)
{
	if(!mPopulated) {
		for(unsigned int i=0; i<mNext.size(); i++) {
			// Get the first token in the stream
			mNext[i] = getNext();
		}
		mPopulated = true;
	}

	return mNext[num];
}

/*!
 * \brief Consume the current token and load the next
 */
void BaseTokenizer::consume()
{
	if(!error() && mNext[0].type != Token::TypeEnd) {
		for(unsigned int i=0; i<mNext.size() - 1; i++) {
			mNext[i] = mNext[i+1];
		}
		mNext[mNext.size() - 1] = getNext();
	}
}

/*!
 * \brief Ensure there are enough characters in the buffer
 * \param length Minimum length of buffer
 * \return True if buffer could be filled up to at least length characters
 */
bool BaseTokenizer::fillBuffer(size_t length)
{
	// Read one character at a time until we either fill the buffer or hit EOF
	while(mBuffer.size() < length) {
		char c;
		mStream.read(&c, 1);
		if(mStream.eof()) {
			break;
		} else {
			mBuffer.push_back(c);
		}
	}

	return mBuffer.size() >= length;
}

/*!
 * \brief Remove characters from the buffer
 * \param length Number of characters to remove
 */
void BaseTokenizer::emptyBuffer(size_t length)
{
	// Remove characters one at a time
	for(size_t i=0; i<length; i++) {
		if(i >= mBuffer.size()) {
			break;
		}

		// Update line and column information
		mColumn++;
		if(mBuffer[i] == '\n') {
			mLine++;
			mColumn = 1;
		}
	}

	mBuffer.erase(0, length);
}

/*!
 * \brief Remove all whitespace from the front of the input buffer
 */
void BaseTokenizer::skipWhitespace()
{
	while(true) {
		// Ensure the buffer has at least one character
		if(!fillBuffer(1)) {
			break;
		}

		// If the front character matches any of the whitespace characters, remove it
		bool found = false;
		for(int i=0; i<sizeof(whitespace)/sizeof(char); i++) {
			if(mBuffer[0] == whitespace[i]) {
				emptyBuffer(1);
				found = true;
				break;
			}
		}

		// Break if the buffer now contains a non-whitespace character at the front
		if(!found) {
			break;
		}
	}
}

/*!
 * \brief Report an error
 * \param message Error message
 */
void BaseTokenizer::setError(const std::string &message)
{
	mError = true;
	mErrorMessage = message;
}

/*!
 * \brief Create a token with the given type and text
 * \param type Token type
 * \param text Token text
 * \return New token
 */
BaseTokenizer::Token BaseTokenizer::createToken(int type, const std::string &text)
{
	Token token;

	token.type = (Token::Type)type;
	token.text = text;
	token.line = mLine;
	token.column = mColumn;

	return token;
}

/*!
 * \brief Utility function to scan a set of literals, and return a token if found
 * \param literals Set of strings to scan for
 * \param numLiterals Number of literals
 * \param token Token to construct
 * \param literalType Type to assign to token if literal is found
 * \return True if literal was found
 */
bool BaseTokenizer::scanLiteral(char *literals[], int numLiterals, Token &token)
{
	// Scan through the list of literals and see if any match
	for(int i=0; i<numLiterals; i++) {
		char *lit = literals[i];
		size_t len = strlen(lit);

		// Ensure that there are a sufficient number of characters in the buffer
		if(!fillBuffer(len)) {
			continue;
		}

		// If the buffer matches, construct a token out of it and remove it from the buffer
		if(buffer().substr(0, len) == lit) {
			token = createToken(Token::TypeLiteral, buffer().substr(0, len));
			emptyBuffer(len);
			return true;
		}
	}

	return false;
}

/*!
 * \brief Constructor
 * \param filename Filename to tokenize
 */
Tokenizer::Tokenizer(const std::string &filename)
: BaseTokenizer(filename, 2)
{
}

/*!
 * \brief Get the next token in the stream
 */
Tokenizer::Token Tokenizer::getNext()
{
	Token next;

	// Start out by moving past any whitespace
	skipWhitespace();

	// Check if we've reached the end of the file
	if(!fillBuffer(1)) {
		next = createToken(Token::TypeEnd, "");
		return next;
	}

	// Scan through the list of literals and see if any match
	if(scanLiteral(literals, sizeof(literals)/sizeof(char*), next)) {
		return next;
	}

	// If no literals matched, see if an identifier can be constructed
	if(std::isalpha(buffer()[0])) {
		size_t len = 0;
		while(std::isalpha(buffer()[len])) {
			len++;
			if(!fillBuffer(len)) {
				break;
			}
		}

		// Construct a token out of the characters found
		next = createToken(TypeIdentifier, buffer().substr(0, len));
		emptyBuffer(len);
		return next;
	}

	// If an identifier couldn't be found, check for a number
	if(std::isdigit(buffer()[0])) {
		size_t len = 0;
		while(std::isdigit(buffer()[len])) {
			len++;
			if(!fillBuffer(len)) {
				break;
			}
		}

		// Construct a token out of the characters found
		next = createToken(TypeNumber, buffer().substr(0, len));
		emptyBuffer(len);
		return next;
	}

	if(buffer()[0] == '\"') {
		size_t len = 2;
		while(true) {
			if(!fillBuffer(len)) {
				setError("Unterminated string literal");
				return next;
			}

			if(buffer()[len - 1] == '\"') {
				break;
			}
			len++;
		}

		// Construct a token out of the characters found
		next = createToken(TypeString, buffer().substr(1, len - 2));
		emptyBuffer(len);
		return next;
	}

	// Nothing matched, log an error
	std::stringstream ss;
	ss << "Illegal symbol '" << buffer()[0] << "'";
	setError(ss.str());

	return next;
}

/*!
 * \brief Convenience function for retrieving name of a token type for printing
 * \param Token type
 * \return Printable name for the type of token
 */
std::string Tokenizer::typeName(TokenType type)
{
	switch(type) {
		case TypeLiteral:
			return "<literal>";
		case TypeIdentifier:
			return "<identifier>";
		case TypeNumber:
			return "<number>";
		case TypeEnd:
			return "<end>";
	}

	return "";
}

}
