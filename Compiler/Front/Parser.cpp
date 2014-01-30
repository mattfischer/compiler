#include "Front/Parser.h"

#include "Front/Type.h"

#include <sstream>
#include <cctype>
#include <exception>

#include <string.h>

namespace Front {

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

/*!
 * \brief Constructor.
 * \param tokenizer Input token source
 */
Parser::Parser(Tokenizer &tokenizer)
	: mTokenizer(tokenizer)
{
	mError = false;
}

/*!
 * \brief Get the next token in the stream
 * \param Token number
 * \return Next token
 */
const Tokenizer::Token &Parser::next(int num)
{
	return mTokenizer.next(num);
}

/*!
 * \brief Consume the next token if it matches the given type, throw an error if it doesn't
 * \param type Type of token to expect
 */
void Parser::expect(Tokenizer::Token::Type type)
{
	// If the tokenizer has an error, propagate it up
	if(mTokenizer.error()) {
		throw ParseException(mTokenizer.errorMessage(), next().line, next().column);
	}

	// If the token type doesn't match throw an error
	if(next().type != type) {
		errorExpected(Tokenizer::Token::typeName(type));
	}

	// Otherwise, consume it
	consume();
}

/*!
 * \brief Consume the next token if it is a literal with the given text, throw an error if it is not
 * \param text Text of expected literal
 */
void Parser::expectLiteral(const std::string &text)
{
	// If the tokenizer has an error, propagate it up
	if(mTokenizer.error()) {
		throw ParseException(mTokenizer.errorMessage(), next().line, next().column);
	}

	// If the next token is not a literal, or does not have the expected text, throw an error
	if(next().type != Tokenizer::Token::TypeLiteral || next().text != text) {
		errorExpected(text);
	}

	// Otherwise, consume it
	consume();
}

/*!
 * \brief Check if the next token is of a given type
 * \param type Type of token to match
 * \param num Lookahead number
 * \return True if next token matches
 */
bool Parser::match(Tokenizer::Token::Type type, int num)
{
	// Check if the tokenizer has an error or if the next token is of a different type
	if(mTokenizer.error() || next(num).type != type) {
		return false;
	}

	return true;
}

/*!
 * \brief Check if the next token is a literal with the given text
 * \param text Text of literal to match
 * \param num Lookahead number
 * \return True if next token matches
 */
bool Parser::matchLiteral(const std::string &text, int num)
{
	// Check if the tokenizer has an error or if the next token is a literal with the given text
	if(mTokenizer.error() || next(num).type != Tokenizer::Token::TypeLiteral || next(num).text != text) {
		return false;
	}

	return true;
}

/*!
 * \brief Consume the next tokens
 * \param num Number of tokens to conume
 */
void Parser::consume(int num)
{
	for(int i=0; i<num; i++) {
		mTokenizer.consume();
	}
}

/*!
 * \brief Throw an error stating that expected token was not found.  Never returns.
 * \param expected Expected token/node that was not found in input stream
 */
void Parser::errorExpected(const std::string &expected)
{
	std::stringstream ss;
	ss << "expected " << expected << ", '" << next().text << "' found instead";
	throw ParseException(ss.str(), next().line, next().column);
}

/*!
 * \brief Construct a new tree node with the given type and line
 * \param nodeType Type for new node
 * \param line Line number to associate with node
 * \param nodeSubtype Subtype for new node
 * \return Newly constructed node
 */
Node *Parser::newNode(Node::NodeType nodeType, int line, Node::NodeSubtype nodeSubtype)
{
	// Construct the new node
	Node *node = new Node;
	node->nodeType = nodeType;
	node->nodeSubtype = nodeSubtype;
	node->type = 0;
	node->line = line;

	// Add the node to the list of nodes, so that they can all be deleted if an error occurs
	mNodes.push_back(node);

	return node;
}

/*!
 * \brief Parse the input stream
 * \return Abstract syntax tree representing parse, or 0 if an error occurred
 */
Node *Parser::parse()
{
	try {
		// Parse the stream
		return parseProgram();
	} catch(ParseException parseException) {
		// Collect the error information from the exception
		mError = true;
		mErrorMessage = parseException.message();
		mErrorLine = parseException.line();
		mErrorColumn = parseException.column();

		// Delete all nodes created along the way
		for(unsigned int i=0; i<mNodes.size(); i++) {
			delete mNodes[i];
		}
		return 0;
	}
}

Node *Parser::parseProgram()
{
	// <Program> := <ProcedureList> END
	Node *node = parseProcedureList();
	expect(Tokenizer::Token::TypeEnd);

	return node;
}

Node *Parser::parseProcedureList()
{
	// <ProcedureList> := { <Procedure> }*
	Node *node = newNode(Node::NodeTypeList, next().line);

	Node *procedure;
	while(procedure = parseProcedure()) {
		node->children.push_back(procedure);
	}

	return node;
}

Node *Parser::parseProcedure()
{
	// <Procedure> := <Type> IDENTIFIER '(' <ArgumentDeclarationList> ')' '{' <StatementList> '}'
	Node *returnType = parseType();
	if(!returnType)	return 0;

	Node *node = newNode(Node::NodeTypeProcedureDef, returnType->line);
	node->children.push_back(returnType);
	node->lexVal.s = next().text;
	expect(Tokenizer::Token::TypeIdentifier);

	expectLiteral("(");
	node->children.push_back(parseArgumentDeclarationList());
	expectLiteral(")");

	expectLiteral("{");
	node->children.push_back(parseStatementList());
	expectLiteral("}");

	return node;
}

Node *Parser::parseArgumentDeclarationList()
{
	// <ArgumentDeclarationList> := <VariableDeclaration> { ',' <VariableDeclaration> }*
	Node *node = newNode(Node::NodeTypeList, next().line);

	Node *argument;
	while(argument = parseVariableDeclaration()) {
		node->children.push_back(argument);
		if(matchLiteral(",")) {
			consume();
		} else {
			break;
		}
	}

	return node;
}

Node *Parser::parseVariableDeclaration()
{
	// <VariableDeclaration> := <Type> IDENTIFIER
	Node *type = parseType();
	if(!type) return 0;

	Node *node = newNode(Node::NodeTypeVarDecl, type->line);
	node->lexVal.s = next().text;
	expect(Tokenizer::Token::TypeIdentifier);

	node->children.push_back(type);

	return node;
}

Node *Parser::parseType(bool required)
{
	Node *node;

	if(match(Tokenizer::Token::TypeIdentifier)) {
		if(Type::find(next().text)) {
			node = parseIdentifier();
			if(matchLiteral("[") && matchLiteral("]", 1)) {
				consume(2);
				Node *arrayNode = newNode(Node::NodeTypeArray, node->line);
				arrayNode->children.push_back(node);
				node = arrayNode;
			}
			return node;
		}
	}

	// Throw an error if a type was required and none was found
	if(required) {
		errorExpected("<type>");
	}

	return 0;
}

Node *Parser::parseStatementList()
{
	// <StatementList> := { <Statement> }*
	Node *node = newNode(Node::NodeTypeList, next().line);

	Node *statement;
	while(statement = parseStatement()) {
		node->children.push_back(statement);
	}

	return node;
}

Node *Parser::parseStatement(bool required)
{
	Node *node;

	if(node = parseVariableDeclaration()) {
		// <Statement> := <VariableDeclaration> { = <Expression> ';' }?
		if(matchLiteral("=")) {
			consume();
			Node *assign = newNode(Node::NodeTypeAssign, node->line);
			assign->children.push_back(node);
			assign->children.push_back(parseExpression(true));
			node = assign;
		}
		expectLiteral(";");

		return node;
	} else if(node = parseExpression()) {
		// <Statement> := <Expression> ';'
		expectLiteral(";");

		return node;
	} else if(matchLiteral("return")) {
		// <Statement> := 'return' <Expression> ';'
		node = newNode(Node::NodeTypeReturn, next().line);
		consume();

		node->children.push_back(parseExpression(true));
		expectLiteral(";");

		return node;
	} else if(matchLiteral("print")) {
		// <Statement> := 'print' <Expression> ';'
		node = newNode(Node::NodeTypePrint, next().line);
		consume();

		node->children.push_back(parseExpression(true));
		expectLiteral(";");

		return node;
	} else if(matchLiteral("if")) {
		// <Statement> := 'if' '(' <Expression> ')' <Clause> { 'else' <Clause> }?
		node = newNode(Node::NodeTypeIf, next().line);
		consume();

		expectLiteral("(");
		node->children.push_back(parseExpression(true));
		expectLiteral(")");

		node->children.push_back(parseClause(true));
		if(matchLiteral("else")) {
			consume();
			node->children.push_back(parseClause(true));
		}

		return node;
	} else if(matchLiteral("while")) {
		// <Statement> := 'while' '(' <Expression> ')' <Clause>
		node = newNode(Node::NodeTypeWhile, next().line);
		consume();

		expectLiteral("(");
		node->children.push_back(parseExpression(true));
		expectLiteral(")");

		node->children.push_back(parseClause(true));

		return node;
	}

	// Throw an error if a statement was required and none was found
	if(required) {
		errorExpected("<statement>");
	}

	return 0;
}

Node *Parser::parseClause(bool required)
{
	if(matchLiteral("{")) {
		// <Clause> := '{' <StatementList> '}'
		consume();

		Node *node = parseStatementList();
		expectLiteral("}");

		return node;
	} else {
		// <Clause> := <Statement>
		return parseStatement(required);
	}
}

Node *Parser::parseExpression(bool required)
{
	// <Expression> := <CompareExpression> { '=' <CompareExpression> }*
	Node *node = parseCompareExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("=")) {
			consume();

			Node *assignNode = newNode(Node::NodeTypeAssign, node->line);
			assignNode->children.push_back(node);
			assignNode->children.push_back(parseCompareExpression(true));
			node = assignNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseCompareExpression(bool required)
{
	// <CompareExpression> := <AddExpression> { [ '==' | '!=' ] <AddExpression> }*
	Node *node = parseAddExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("==") || matchLiteral("!=")) {
			Node::NodeSubtype subtype;
			if(matchLiteral("==")) subtype = Node::NodeSubtypeEqual;
			else if(matchLiteral("!=")) subtype = Node::NodeSubtypeNequal;
			consume();

			Node *compareNode = newNode(Node::NodeTypeCompare, node->line, subtype);
			compareNode->children.push_back(node);
			compareNode->children.push_back(parseAddExpression(true));
			node = compareNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseAddExpression(bool required)
{
	// <AddExpression> := <MultiplyExpression> { '+' <MultiplyExpression> }*
	Node *node = parseMultiplyExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("+")) {
			consume();

			Node *addNode = newNode(Node::NodeTypeArith, node->line, Node::NodeSubtypeAdd);
			addNode->children.push_back(node);
			addNode->children.push_back(parseMultiplyExpression(true));
			node = addNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseMultiplyExpression(bool required)
{
	// <MultiplyExpression> := <FunctionExpression> { '*' <FunctionExpression> }*
	Node *node = parseFunctionExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("*")) {
			consume();

			Node *multiplyNode = newNode(Node::NodeTypeArith, node->line, Node::NodeSubtypeMultiply);
			multiplyNode->children.push_back(node);
			multiplyNode->children.push_back(parseFunctionExpression(true));
			node = multiplyNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseFunctionExpression(bool required)
{
	// <FunctionExpression> := <BaseExpression> { '(' <ExpressionList> ')' }*
	Node *node = parseBaseExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("(")) {
			consume();

			Node *callNode = newNode(Node::NodeTypeCall, node->line);
			callNode->children.push_back(node);
			callNode->children.push_back(parseExpressionList());
			expectLiteral(")");
			node = callNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseBaseExpression(bool required)
{
	Node *node;

	if(match(Tokenizer::Token::TypeNumber)) {
		// <BaseExpression> := NUMBER
		node = newNode(Node::NodeTypeConstant, next().line);
		node->lexVal.i = std::atoi(next().text.c_str());
		consume();

		return node;
	} else if(matchLiteral("(")) {
		// <BaseExpression> := '(' <Expression> ')'
		consume();

		node = parseExpression(true);
		expectLiteral(")");

		return node;
	} else if(node = parseIdentifier()) {
		// <BaseExpression> := <Identifier>
		return node;
	} else if(matchLiteral("new")) {
		// <BaseExpression> := 'new' <Type> { '[' <Expression> ']' }?
		node = newNode(Node::NodeTypeNew, next().line);
		consume();

		Node *type = parseType(true);
		if(matchLiteral("[")) {
			consume();

			Node *count = parseExpression(true);
			expectLiteral("]");
			Node *arrayType = newNode(Node::NodeTypeArray, type->line);
			arrayType->children.push_back(type);
			arrayType->children.push_back(count);
			type = arrayType;
		}

		node->children.push_back(type);
		return node;
	}

	// Throw an error if a base expression was required and none was found
	if(required) {
		errorExpected("<base-expression>");
	}

	return 0;
}

Node *Parser::parseExpressionList()
{
	// <ExpressionList> := <Expression> { ',' <Expression> }*
	Node *node = newNode(Node::NodeTypeList, next().line);
	Node *expression;
	while(expression = parseExpression()) {
		node->children.push_back(expression);
		if(matchLiteral(",")) {
			consume();
		} else {
			break;
		}
	}

	return node;
}

Node *Parser::parseIdentifier()
{
	// <Identifier> := IDENTIFIER
	if(match(Tokenizer::Token::TypeIdentifier))
	{
		Node *node = newNode(Node::NodeTypeId, next().line);
		node->lexVal.s = mTokenizer.next().text;
		consume();

		return node;
	}

	return 0;
}

}
