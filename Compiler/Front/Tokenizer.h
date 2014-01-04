#ifndef FRONT_TOKENIZER_H
#define FRONT_TOKENIZER_H

#include <string>
#include <fstream>

namespace Front {
class Tokenizer {
public:
	Tokenizer(const std::string &filename);

	struct Token {
		enum Type {
			TypeLiteral,
			TypeIdentifier,
			TypeNumber,
			TypeEnd
		};

		Type type;
		std::string text;
		int line;
		int column;
	};

	const Token &next() { return mNext; }

	void consume();

private:
	std::ifstream mStream;

	Token mNext;
	std::string mBuffer;
	int mLine;
	int mColumn;

	void getNext();
	bool fillBuffer(size_t length);
	void emptyBuffer(size_t length);
	void skipWhitespace();
};
}
#endif
