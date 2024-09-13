#ifndef FRONT_PARSER_H
#define FRONT_PARSER_H

#include "Input/Parser.h"

#include "Front/HllTokenizer.h"
#include "Front/Node.h"

namespace Front {
/*!
 * \brief Parser for the compiler
 *
 * Consumes a token list from the tokenizer and converts it to an Abstract Syntax Tree.
 * The parser is a hand-coded recursive descent parser.
 */
class HllParser : public Input::Parser {
public:
	HllParser(HllTokenizer &tokenizer);

	std::unique_ptr<Node> parse();

private:
	HllTokenizer &mHllTokenizer; //!< Reference to the tokenizer being used

	std::unique_ptr<Node> newNode(Node::Type nodeType, int line, Node::Subtype nodeSubtype = Node::Subtype::None);

	std::unique_ptr<Node> parseProgram();
	std::unique_ptr<Node> parseProcedure();
	std::unique_ptr<Node> parseStruct();
	std::unique_ptr<Node> parseClass();
	std::unique_ptr<Node> parseClassMember();
	std::unique_ptr<Node> parseVariableDeclaration();
	std::unique_ptr<Node> parseArgumentList();
	std::unique_ptr<Node> parseType(bool required = false);
	std::unique_ptr<Node> parseStatementList();
	std::unique_ptr<Node> parseStatement(bool required = false);
	std::unique_ptr<Node> parseClause(bool required = false);
	std::unique_ptr<Node> parseExpression(bool required = false);
	std::unique_ptr<Node> parseOrExpression(bool required = false);
	std::unique_ptr<Node> parseAndExpression(bool required = false);
	std::unique_ptr<Node> parseCompareExpression(bool required = false);
	std::unique_ptr<Node> parseAddExpression(bool required = false);
	std::unique_ptr<Node> parseMultiplyExpression(bool required = false);
	std::unique_ptr<Node> parseSuffixExpression(bool required = false);
	std::unique_ptr<Node> parseBaseExpression(bool required = false);
	std::unique_ptr<Node> parseExpressionList();
};
}
#endif
