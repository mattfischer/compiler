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
	void expect(Tokenizer::Token::Type type);
	void expectLiteral(const std::string &text);
	bool match(Tokenizer::Token::Type type);
	bool matchLiteral(const std::string &text);
	void errorExpected(const std::string &expected);
	void consume();

	std::vector<Node*> mNodes;

	Node *newNode(Node::NodeType nodeType, int line, Node::NodeSubtype nodeSubtype = Node::NodeSubtypeNone);

	Node *parseProgram();
	Node *parseProcedureList();
	Node *parseProcedure();
	Node *parseArgumentDeclarationList();
	Node *parseVariableDeclaration();
	Node *parseType();
	Node *parseStatementList();
	Node *parseStatement(bool required = false);
	Node *parseClause(bool required = false);
	Node *parseExpression(bool required = false);
	Node *parseCompareExpression(bool required = false);
	Node *parseAddExpression(bool required = false);
	Node *parseMultiplyExpression(bool required = false);
	Node *parseFunctionExpression(bool required = false);
	Node *parseBaseExpression(bool required = false);
	Node *parseExpressionList();
	Node *parseIdentifier();
};
}
#endif
