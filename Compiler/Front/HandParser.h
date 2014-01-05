#ifndef FRONT_HAND_PARSER_H
#define FRONT_HAND_PARSER_H

#include "Front/Tokenizer.h"
#include "Front/SyntaxNode.h"

namespace Front {
class HandParser {
public:
	HandParser(Tokenizer &tokenizer);

	SyntaxNode *parse();

	bool error() { return mError; }

private:
	Tokenizer &mTokenizer;
	bool mError;
	std::string mErrorMessage;

	const Tokenizer::Token &next();
	bool expect(Tokenizer::Token::Type type);
	bool expectLiteral(const std::string &text);
	bool match(Tokenizer::Token::Type type);
	bool matchLiteral(const std::string &text);
	void errorExpected(const std::string &expected);
	void consume();

	std::vector<SyntaxNode*> mNodes;

	SyntaxNode *newNode(SyntaxNode::NodeType nodeType, SyntaxNode::NodeSubtype nodeSubtype = SyntaxNode::NodeSubtypeNone);

	SyntaxNode *parseProcedureList();
	SyntaxNode *parseProcedure();
	SyntaxNode *parseArgumentDeclarationList();
	SyntaxNode *parseVariableDeclaration();
	SyntaxNode *parseType();
	SyntaxNode *parseStatementList();
	SyntaxNode *parseStatement();
	SyntaxNode *parsePrintStatement();
	SyntaxNode *parseReturnStatement();
	SyntaxNode *parseIfStatement();
	SyntaxNode *parseWhileStatement();
	SyntaxNode *parseClause();
	SyntaxNode *parseExpression();
	SyntaxNode *parseAddExpression();
	SyntaxNode *parseMultiplyExpression();
	SyntaxNode *parseBaseExpression();
	SyntaxNode *parseExpressionList();
	SyntaxNode *parseIdentifier();
};
}
#endif
