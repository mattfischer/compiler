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
 * \param filename Filename to tokenize
 */
Tokenizer::Tokenizer(const std::string &filename)
: mStream(filename.c_str())
{
	mLine = 1;
	mColumn = 1;
	mError = false;

	for(int i=0; i<NumLookahead; i++) {
		// Get the first token in the stream
		mNext[i] = getNext();
	}
}

/*!
 * \brief Consume the current token and load the next
 */
void Tokenizer::consume()
{
	if(!mError && mNext[0].type != Token::TypeEnd) {
		for(int i=0; i<NumLookahead - 1; i++) {
			mNext[i] = mNext[i+1];
		}
		mNext[NumLookahead - 1] = getNext();
	}
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
		next.type = Token::TypeEnd;
		next.text = "";
		next.line = mLine;
		next.column = mColumn;
		return next;
	}

	// Scan through the list of literals and see if any match
	for(int i=0; i<sizeof(literals)/sizeof(char*); i++) {
		char *lit = literals[i];
		size_t len = strlen(lit);

		// Ensure that there are a sufficient number of characters in the buffer
		if(!fillBuffer(len)) {
			continue;
		}

		// If the buffer matches, construct a token out of it and remove it from the buffer
		if(mBuffer.substr(0, len) == lit) {
			next.type = Token::TypeLiteral;
			next.text = mBuffer.substr(0, len);
			next.line = mLine;
			next.column = mColumn;
			emptyBuffer(len);
			return next;
		}
	}

	// If no literals matched, see if an identifier can be constructed
	if(std::isalpha(mBuffer[0])) {
		size_t len = 0;
		while(std::isalpha(mBuffer[len])) {
			len++;
			if(!fillBuffer(len)) {
				break;
			}
		}

		// Construct a token out of the characters found
		next.type = Token::TypeIdentifier;
		next.text = mBuffer.substr(0, len);
		next.line = mLine;
		next.column = mColumn;
		emptyBuffer(len);
		return next;
	}

	// If an identifier couldn't be found, check for a number
	if(std::isdigit(mBuffer[0])) {
		size_t len = 0;
		while(std::isdigit(mBuffer[len])) {
			len++;
			if(!fillBuffer(len)) {
				break;
			}
		}

		// Construct a token out of the characters found
		next.type = Token::TypeNumber;
		next.text = mBuffer.substr(0, len);
		next.line = mLine;
		next.column = mColumn;
		emptyBuffer(len);
		return next;
	}

	// Nothing matched, log an error
	mError = true;
	std::stringstream ss;
	ss << "Illegal symbol '" << mBuffer[0] << "'";
	mErrorMessage = ss.str();

	return next;
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
 * \brief Remove all whitespace from the front of the input buffer
 */
void Tokenizer::skipWhitespace()
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
 * \brief Convenience function for retrieving name of a token type for printing
 * \param Token type
 * \return Printable name for the type of token
 */
std::string Tokenizer::Token::typeName(Type type)
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
