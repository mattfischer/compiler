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

	Node *parse();

private:
	std::vector<Node*> mNodes; //!< List of all nodes created so far
	HllTokenizer &mHllTokenizer; //!< Reference to the tokenizer being used

	Node *newNode(Node::NodeType nodeType, int line, Node::NodeSubtype nodeSubtype = Node::NodeSubtypeNone);

	Node *parseProgram();
	Node *parseProcedure();
	Node *parseStruct();
	Node *parseClass();
	Node *parseClassMember();
	Node *parseVariableDeclaration();
	Node *parseArgumentList();
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
	Node *parseExpressionList();
};
}
#endif
