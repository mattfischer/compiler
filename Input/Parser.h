#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include "Input/Tokenizer.h"

#include <exception>

namespace Input {

class Parser {
public:
	Parser(Tokenizer &tokenizer);

	bool error() { return mError; } //!< True if an error occurred
	const std::string &errorMessage() { return mErrorMessage; } //!< Error message, if any
	int errorLine() { return mErrorLine; } //!< Line where error occurred
	int errorColumn() { return mErrorColumn; } //!< Column where error occurred

	/*!
	 * \brief Exception thrown when a parse error occurs
	 */
	class ParseException : public std::exception
	{
	public:
		ParseException(const std::string &message, int line, int column)
			: mMessage(message), mLine(line), mColumn(column)
		{}

		const char *what() { return mMessage.c_str(); } //!< Standard exception message function
		const std::string &message() { return mMessage; } //!< String version of message
		int line() { return mLine; } //!< Line of error
		int column() { return mColumn; } //!< Column of error

	private:
		std::string mMessage; //!< Message
		int mLine; //!< Line
		int mColumn; //!< Column
	};

protected:
	const Input::Tokenizer::Token &next(int num = 0);
	void expect(int type);
	void expectLiteral(const std::string &text);
	bool match(int type, int num = 0);
	bool matchLiteral(const std::string &text, int num = 0);
	void errorExpected(const std::string &expected);
	void consume(int num = 1);
	void setError(const std::string &message, int line, int column);

private:
	Tokenizer &mTokenizer; //!< Tokenizer providing input token stream
	bool mError; //!< True if error occurred
	std::string mErrorMessage; //!< Error message
	int mErrorLine; //!< Error line
	int mErrorColumn; //!< Error column

};

}
#endif