#include "Front/Tokenizer.h"

#include <cctype>
#include <sstream>

char whitespace[] = { ' ', '\t', '\r', '\n' };
char *literals[] = { "print", "if", "else", "while", "return", "==", "!=", "+", "*", "(", ")", "=", ";", "{", "}", "," };

namespace Front {
Tokenizer::Tokenizer(const std::string &filename)
: mStream(filename.c_str())
{
	mLine = 0;
	mColumn = 0;
	mError = false;

	getNext();
}

void Tokenizer::consume()
{
	if(!mError && mNext.type != Token::TypeEnd) {
		getNext();
	}
}

void Tokenizer::getNext()
{
	skipWhitespace();

	if(!fillBuffer(1)) {
		mNext.type = Token::TypeEnd;
		mNext.text = "";
		mNext.line = mLine;
		mNext.column = mColumn;
		return;
	}

	for(int i=0; i<sizeof(literals)/sizeof(char*); i++) {
		char *lit = literals[i];
		size_t len = strlen(lit);

		if(!fillBuffer(len)) {
			continue;
		}

		if(mBuffer.substr(0, len) == lit) {
			mNext.type = Token::TypeLiteral;
			mNext.text = mBuffer.substr(0, len);
			mNext.line = mLine;
			mNext.column = mColumn;
			emptyBuffer(len);
			return;
		}
	}

	if(std::isalpha(mBuffer[0])) {
		size_t len = 0;
		while(std::isalpha(mBuffer[len])) {
			len++;
			if(!fillBuffer(len)) {
				break;
			}
		}

		mNext.type = Token::TypeIdentifier;
		mNext.text = mBuffer.substr(0, len);
		mNext.line = mLine;
		mNext.column = mColumn;
		emptyBuffer(len);
		return;
	}

	if(std::isdigit(mBuffer[0])) {
		size_t len = 0;
		while(std::isdigit(mBuffer[len])) {
			len++;
			if(!fillBuffer(len)) {
				break;
			}
		}

		mNext.type = Token::TypeNumber;
		mNext.text = mBuffer.substr(0, len);
		mNext.line = mLine;
		mNext.column = mColumn;
		emptyBuffer(len);
		return;
	}

	mError = true;
	std::stringstream ss;
	ss << "Illegal symbol '" << mBuffer[0] << "'";
	mErrorMessage = ss.str();
}

bool Tokenizer::fillBuffer(size_t length)
{
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

void Tokenizer::emptyBuffer(size_t length)
{
	for(size_t i=0; i<length; i++) {
		if(i >= mBuffer.size()) {
			break;
		}

		mColumn++;
		if(mBuffer[i] == '\n') {
			mLine++;
			mColumn = 0;
		}
	}

	mBuffer.erase(0, length);
}

void Tokenizer::skipWhitespace()
{
	while(1) {
		if(!fillBuffer(1)) {
			break;
		}

		bool found = false;
		for(int i=0; i<sizeof(whitespace)/sizeof(char); i++) {
			if(mBuffer[0] == whitespace[i]) {
				emptyBuffer(1);
				found = true;
				break;
			}
		}

		if(!found) {
			break;
		}
	}
}

std::string Tokenizer::Token::typeName(Type type)
{
	switch(type) {
		case TypeLiteral:
			return "literal";
		case TypeIdentifier:
			return "identifier";
		case TypeNumber:
			return "number";
		case TypeEnd:
			return "end";
	}

	return "";
}

}
