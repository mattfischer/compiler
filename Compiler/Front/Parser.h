#ifndef FRONT_PARSER_H
#define FRONT_PARSER_H

#include "Front/Tokenizer.h"
#include "Front/Node.h"

namespace Front {
/*!
 * \brief Parser for the compiler
 *
 * Consumes a token list from the tokenizer and converts it to an Abstract Syntax Tree.
 * The parser is a hand-coded recursive descent parser.
 */
class Parser {
public:
	Parser(Tokenizer &tokenizer);

	Node *parse();

	bool error() { return mError; } //!< True if an error occurred
	const std::string &errorMessage() { return mErrorMessage; } //!< Error message, if any
	int errorLine() { return mErrorLine; } //!< Line where error occurred
	int errorColumn() { return mErrorColumn; } //!< Column where error occurred

private:
	Tokenizer &mTokenizer; //!< Tokenizer providing input token stream
	bool mError; //!< True if error occurred
	std::string mErrorMessage; //!< Error message
	int mErrorLine; //!< Error line
	int mErrorColumn; //!< Error column

	const Tokenizer::Token &next(int num = 0);
	void expect(Tokenizer::TokenType type);
	void expectLiteral(const std::string &text);
	bool match(Tokenizer::TokenType type, int num = 0);
	bool matchLiteral(const std::string &text, int num = 0);
	void errorExpected(const std::string &expected);
	void consume(int num = 1);

	std::vector<Node*> mNodes; //!< List of all nodes created so far
	std::vector<std::string> mTypeNames; //!< List of all type names

	Node *newNode(Node::NodeType nodeType, int line, Node::NodeSubtype nodeSubtype = Node::NodeSubtypeNone);

	Node *parseProgram();
	Node *parseProcedure();
	Node *parseStruct();
	Node *parseVariableDeclaration();
	Node *parseType(bool required = false);
	Node *parseStatementList();
	Node *parseStatement(bool required = false);
	Node *parseClause(bool required = false);
	Node *parseExpression(bool required = false);
	Node *parseOrExpression(bool required = false);
	Node *parseAndExpression(bool required = false);
	Node *parseCompareExpression(bool required = false);
	Node *parseAddExpression(bool required = false);
	Node *parseMultiplyExpression(bool required = false);
	Node *parseSuffixExpression(bool required = false);
	Node *parseBaseExpression(bool required = false);
};
}
#endif
