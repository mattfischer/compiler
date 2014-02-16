#include "Input/Tokenizer.h"

namespace Input {
/*!
 * \brief Constructor
 * \param filename Filename to read
 * \param lookahead Number of lookahead tokens
 */
Tokenizer::Tokenizer(const std::string &filename, int lookahead)
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
const Tokenizer::Token &Tokenizer::next(int num)
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
void Tokenizer::consume()
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
bool Tokenizer::fillBuffer(size_t length)
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
void Tokenizer::emptyBuffer(size_t length)
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
 * \brief Skip all characters in a given list from the front of the input buffer
 * \param characters Characters to skip
 * \param numCharacters Number of characters in list
 */
void Tokenizer::skipCharacters(const char characters[], int numCharacters)
{
	while(true) {
		// Ensure the buffer has at least one character
		if(!fillBuffer(1)) {
			break;
		}

		// If the front character matches any of the characters in the list, remove it
		bool found = false;
		for(int i=0; i<numCharacters; i++) {
			if(mBuffer[0] == characters[i]) {
				emptyBuffer(1);
				found = true;
				break;
			}
		}

		// Break if the buffer now contains a character not in the list at the front
		if(!found) {
			break;
		}
	}
}

/*!
 * \brief Report an error
 * \param message Error message
 */
void Tokenizer::setError(const std::string &message)
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
Tokenizer::Token Tokenizer::createToken(int type, const std::string &text)
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
bool Tokenizer::scanLiteral(char *literals[], int numLiterals, Token &token)
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

}