#include "Front/HllTokenizer.h"

#include <cctype>
#include <sstream>

static char whitespace[] = { ' ', '\t', '\r', '\n' };

static char *literals[] = { "==", "!=", ">=", "<=", "++", "--", "&&", "||",
					 ">", "<", "+", "-", "*", "/", "%", "(", ")", "=", ";", "{", "}", ",", "[", "]", ".",
					};

static char *keywords[] = { "print", "if", "else", "while", "return", "new", "for", "break",
					 "continue", "true", "false", "struct"
					};

namespace Front {
/*!
 * \brief Constructor
 * \param filename Filename to tokenize
 */
HllTokenizer::HllTokenizer(std::istream &stream)
: Input::Tokenizer(stream, 2)
{
}

struct {
	char escape;
	char value;
} escapes[] = {
	{ '0', '\0' },
	{ 'n', '\n' },
	{ '\\', '\\' },
	{ '\"', '\"' }
};

bool HllTokenizer::evaluateEscapes(std::string &text)
{
	for(unsigned int i=0; i<text.size(); i++) {
		if(text[i] == '\\') {
			bool valid = false;
			if(i < text.size() - 1) {
				for(unsigned int j=0; j < sizeof(escapes) / sizeof(escapes[0]); j++) {
					if(text[i+1] == escapes[j].escape) {
						text[i] = escapes[j].value;
						text.erase(i+1, 1);
						valid = true;
						break;
					}
				}
			}

			if(!valid) {
				setError("Invalid escape sequence");
				return false;
			}
		}
	}

	return true;
}

/*!
 * \brief Get the next token in the stream
 */
HllTokenizer::Token HllTokenizer::getNext()
{
	Token next;

	// Start out by moving past any whitespace
	skipCharacters(whitespace, sizeof(whitespace) / sizeof(char));

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
	if(std::isalpha(buffer()[0]) || buffer()[0] == '_') {
		size_t len = 0;
		while(std::isalpha(buffer()[len]) || buffer()[len] == '_') {
			len++;
			if(!fillBuffer(len + 1)) {
				break;
			}
		}

		TokenType type = TypeIdentifier;
		std::string string = buffer().substr(0, len);

		// Check if the string is a keyword
		for(int i=0; i<sizeof(keywords) / sizeof(char*); i++) {
			if(string == keywords[i]) {
				type = TypeLiteral;
				break;
			}
		}

		// Construct a token out of the characters found
		next = createToken(type, string);
		emptyBuffer(len);
		return next;
	}

	// If an identifier couldn't be found, check for a number
	if(std::isdigit(buffer()[0])) {
		size_t len = 0;
		while(std::isdigit(buffer()[len])) {
			len++;
			if(!fillBuffer(len + 1)) {
				break;
			}
		}

		// Construct a token out of the characters found
		next = createToken(TypeNumber, buffer().substr(0, len));
		emptyBuffer(len);
		return next;
	}

	if(buffer()[0] == '\"') {
		size_t len = 1;
		while(true) {
			if(!fillBuffer(len + 1)) {
				setError("Unterminated string literal");
				return next;
			}

			if(buffer()[len] == '\"') {
				break;
			}
			len++;
		}

		// Construct a token out of the characters found
		std::string text = buffer().substr(1, len - 1);
		emptyBuffer(len + 1);

		if(evaluateEscapes(text)) {
			next = createToken(TypeString, text);
		}
		return next;
	}

	if(buffer()[0] == '\'') {
		size_t len = 1;
		while(true) {
			if(!fillBuffer(len + 1)) {
				setError("Unterminated character literal");
				return next;
			}

			if(buffer()[len] == '\'') {
				break;
			}
			len++;
		}

		// Construct a token out of the characters found
		std::string text = buffer().substr(1, len - 1);
		emptyBuffer(len + 1);

		if(evaluateEscapes(text)) {
			if(text.size() == 1) {
				next = createToken(TypeChar, text);
			} else {
				setError("Invalid character literal");
			}
		}
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
std::string HllTokenizer::typeName(int type)
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
