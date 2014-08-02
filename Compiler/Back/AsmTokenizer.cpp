#include "Back/AsmTokenizer.h"

#include "Util/Array.h"

#include <cctype>
#include <sstream>

static char whitespace[] = { ' ', '\t', '\r', '\n' };

static char *literals[] = { "[", "]", ",", ":", "#", "{", "}", "-" };

static char *keywords[] = { "jmp", "add", "sub", "mov", "mult", "div", "mod", "ldr", "str",
							"new", "cmov", "cadd", "ncmov", "ncadd", "equ",
							"neq", "lt", "lte", "gt", "gte", "or", "and", "call", "calli",
							"ldm", "stm", "defproc", "defdata", "string", "lea", "ldb", "stb", "addr",
						  };

namespace Back {
/*!
 * \brief Constructor
 * \param filename Filename to tokenize
 */
AsmTokenizer::AsmTokenizer(std::istream &stream)
: Input::Tokenizer(stream, 2)
{
}

/*!
 * \brief Get the next token in the stream
 */
AsmTokenizer::Token AsmTokenizer::getNext()
{
	Token next;

	// Start out by moving past any whitespace
	skipCharacters(whitespace, N_ELEMENTS(whitespace));

	// Check if we've reached the end of the file
	if(!fillBuffer(1)) {
		next = createToken(Token::TypeEnd, "");
		return next;
	}

	// Scan through the list of literals and see if any match
	if(scanLiteral(literals, N_ELEMENTS(literals), next)) {
		return next;
	}

	// If no literals matched, see if an identifier can be constructed
	if(std::isalpha(buffer()[0]) || buffer()[0] == '_' || buffer()[0] == '.') {
		size_t len = 0;
		while(std::isalnum(buffer()[len]) || buffer()[len] == '_' || buffer()[len] == '.' || buffer()[len] == '$') {
			len++;
			if(!fillBuffer(len + 1)) {
				break;
			}
		}

		TokenType type = TypeIdentifier;
		std::string string = buffer().substr(0, len);

		// Check if the string is a keyword
		for(int i=0; i<N_ELEMENTS(keywords); i++) {
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
		next = createToken(TypeString, buffer().substr(1, len - 1));
		emptyBuffer(len + 1);
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
std::string AsmTokenizer::typeName(int type)
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
