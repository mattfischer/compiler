#ifndef FRONT_PARSER_H
#define FRONT_PARSER_H

#include "Front/Tokenizer.h"
#include "Front/Node.h"

namespace Front {
class Parser {
public:
	Parser(Tokenizer &tokenizer);

	Node *parse();

	bool error() { return mError; }
	const std::string &errorMessage() { return mErrorMessage; }
	int errorLine() { return mErrorLine; }
	int errorColumn() { return mErrorColumn; }

private:
	Tokenizer &mTokenizer;
	bool mError;
	std::string mErrorMessage;
	int mErrorLine;
	int mErrorColumn;

	const Tokenizer::Token &next();
	bool expect(Tokenizer::Token::Type type);
	bool expectLiteral(const std::string &text);
	bool match(Tokenizer::Token::Type type);
	bool matchLiteral(const std::string &text);
	void errorExpected(const std::string &expected);
	void consume();

	std::vector<Node*> mNodes;

	Node *newNode(Node::NodeType nodeType, Node::NodeSubtype nodeSubtype = Node::NodeSubtypeNone);

	Node *parseProcedureList();
	Node *parseProcedure();
	Node *parseArgumentDeclarationList();
	Node *parseVariableDeclaration();
	Node *parseType();
	Node *parseStatementList();
	Node *parseStatement();
	Node *parsePrintStatement();
	Node *parseReturnStatement();
	Node *parseIfStatement();
	Node *parseWhileStatement();
	Node *parseClause();
	Node *parseExpression();
	Node *parseAddExpression();
	Node *parseMultiplyExpression();
	Node *parseBaseExpression();
	Node *parseExpressionList();
	Node *parseIdentifier();
};
}
#endif
