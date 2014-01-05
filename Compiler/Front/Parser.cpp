#include "Front/Parser.h"

#include "Front/Type.h"

#include <sstream>
#include <cctype>
#include <exception>

#include <string.h>

namespace Front {

class ParseException : public std::exception
{
public:
	ParseException(const std::string &message, int line, int column)
		: mMessage(message), mLine(line), mColumn(column)
	{}

	const char *what() { return mMessage.c_str(); }
	const std::string &message() { return mMessage; }
	int line() { return mLine; }
	int column() { return mColumn; }

private:
	std::string mMessage;
	int mLine;
	int mColumn;
};

Parser::Parser(Tokenizer &tokenizer)
	: mTokenizer(tokenizer)
{
	mError = false;
}

const Tokenizer::Token &Parser::next()
{
	return mTokenizer.next();
}

void Parser::expect(Tokenizer::Token::Type type)
{
	if(mTokenizer.error()) {
		throw ParseException(mTokenizer.errorMessage(), next().line, next().column);
	}

	if(next().type != type) {
		errorExpected(Tokenizer::Token::typeName(type));
	}

	consume();
}

void Parser::expectLiteral(const std::string &text)
{
	if(mTokenizer.error()) {
		throw ParseException(mTokenizer.errorMessage(), next().line, next().column);
	}

	if(next().type != Tokenizer::Token::TypeLiteral || next().text != text) {
		errorExpected(text);
	}

	consume();
}

bool Parser::match(Tokenizer::Token::Type type)
{
	if(mTokenizer.error() || next().type != type) {
		return false;
	}

	return true;
}

bool Parser::matchLiteral(const std::string &text)
{
	if(mTokenizer.error() || next().type != Tokenizer::Token::TypeLiteral || next().text != text) {
		return false;
	}

	return true;
}

void Parser::consume()
{
	mTokenizer.consume();
}

void Parser::errorExpected(const std::string &expected)
{
	std::stringstream ss;
	ss << "expected " << expected << ", '" << next().text << "' found instead";
	throw ParseException(ss.str(), next().line, next().column);
}

bool Parser::checkPresent(Node *node, const std::string &name, bool required)
{
	if(!node) {
		if(required) {
			errorExpected(name);
		} else {
			return false;
		}
	}

	return true;
}

Node *Parser::newNode(Node::NodeType nodeType, int line, Node::NodeSubtype nodeSubtype)
{
	Node *node = new Node;
	node->nodeType = nodeType;
	node->nodeSubtype = nodeSubtype;
	node->line = line;

	mNodes.push_back(node);

	return node;
}

Node *Parser::parse()
{
	try {
		return parseProgram();
	} catch(ParseException parseException) {
		mError = true;
		mErrorMessage = parseException.message();
		mErrorLine = parseException.line();
		mErrorColumn = parseException.column();

		for(unsigned int i=0; i<mNodes.size(); i++) {
			delete mNodes[i];
		}
		return 0;
	}
}

Node *Parser::parseProgram()
{
	Node *node = parseProcedureList();
	expect(Tokenizer::Token::TypeEnd);

	return node;
}

Node *Parser::parseProcedureList()
{
	Node *node = newNode(Node::NodeTypeList, next().line);

	Node *procedure;
	while(procedure = parseProcedure()) {
		node->children.push_back(procedure);
	}

	return node;
}

Node *Parser::parseProcedure()
{
	Node *returnType = parseType();
	if(!returnType)	return 0;

	Node *node = newNode(Node::NodeTypeProcedureDef, returnType->line);
	node->children.push_back(returnType);
	node->lexVal._id = strdup(next().text.c_str());
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
	Node *type = parseType();
	if(!type) return 0;

	Node *node = newNode(Node::NodeTypeVarDecl, type->line);
	node->lexVal._id = strdup(next().text.c_str());
	expect(Tokenizer::Token::TypeIdentifier);

	node->children.push_back(type);

	return node;
}

Node *Parser::parseType()
{
	if(match(Tokenizer::Token::TypeIdentifier)) {
		if(Type::find(next().text)) {
			return parseIdentifier();
		}
	}

	return 0;
}

Node *Parser::parseStatementList()
{
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

	if((node = parseVariableDeclaration()) ||
	   (node = parsePrintStatement()) ||
	   (node = parseReturnStatement())) {
		expectLiteral(";");

		return node;
	} else if((node = parseIfStatement()) || (node = parseWhileStatement())) {
		return node;
	} else if(match(Tokenizer::Token::TypeIdentifier)) {
		Node *identifier = parseIdentifier();
		if(matchLiteral("=")) {
			consume();

			node = newNode(Node::NodeTypeAssign, identifier->line);
			node->children.push_back(identifier);
			node->children.push_back(parseExpression(true));
			expectLiteral(";");

			return node;
		} else if(matchLiteral("(")) {
			consume();

			node = newNode(Node::NodeTypeCall, identifier->line);
			node->children.push_back(identifier);
			node->children.push_back(parseExpressionList());
			expectLiteral(")");
			expectLiteral(";");

			return node;
		} else {
			errorExpected("= or (");
		}
	}

	if(required) {
		errorExpected("Statement");
	} else {
		return 0;
	}
}

Node *Parser::parsePrintStatement()
{
	if(matchLiteral("print")) {
		Node *node = newNode(Node::NodeTypePrint, next().line);
		consume();

		node->children.push_back(parseExpression(true));

		return node;
	}

	return 0;
}

Node *Parser::parseReturnStatement()
{
	if(matchLiteral("return")) {
		Node *node = newNode(Node::NodeTypeReturn, next().line);
		consume();

		node->children.push_back(parseExpression(true));

		return node;
	}

	return 0;
}

Node *Parser::parseIfStatement()
{
	if(matchLiteral("if")) {
		Node *node = newNode(Node::NodeTypeIf, next().line);
		consume();

		expectLiteral("(");
		node->children.push_back(parseExpression(true));
		expectLiteral(")");

		node->children.push_back(parseClause(true));
		if(matchLiteral("else")) {
			node->children.push_back(parseClause(true));
		}

		return node;
	}

	return 0;
}

Node *Parser::parseWhileStatement()
{
	if(matchLiteral("while")) {
		Node *node = newNode(Node::NodeTypeWhile, next().line);
		consume();

		expectLiteral("(");
		node->children.push_back(parseExpression(true));
		expectLiteral(")");

		node->children.push_back(parseClause(true));

		return node;
	}

	return 0;
}

Node *Parser::parseClause(bool required)
{
	if(matchLiteral("{")) {
		consume();

		Node *node = parseStatementList();
		expectLiteral("}");

		return node;
	} else {
		return parseStatement(required);
	}
}

Node *Parser::parseExpression(bool required)
{
	Node *arg1 = parseAddExpression();
	if(!checkPresent(arg1, "Expression", required))	{
		return 0;
	}

	if(matchLiteral("==") || matchLiteral("!=")) {
		Node::NodeSubtype subtype;
		if(matchLiteral("==")) subtype = Node::NodeSubtypeEqual;
		else if(matchLiteral("!=")) subtype = Node::NodeSubtypeNequal;
		consume();

		Node *node = newNode(Node::NodeTypeCompare, arg1->line, subtype);

		node->children.push_back(arg1);
		node->children.push_back(parseAddExpression(true));

		return node;
	} else {
		return arg1;
	}
}

Node *Parser::parseAddExpression(bool required)
{
	Node *arg1 = parseMultiplyExpression();
	if(!checkPresent(arg1, "AddExpression", required))	{
		return 0;
	}

	if(matchLiteral("+")) {
		consume();

		Node *node = newNode(Node::NodeTypeArith, arg1->line, Node::NodeSubtypeAdd);
		node->children.push_back(arg1);
		node->children.push_back(parseAddExpression(true));

		return node;
	} else {
		return arg1;
	}
}

Node *Parser::parseMultiplyExpression(bool required)
{
	Node *arg1 = parseBaseExpression();
	if(!checkPresent(arg1, "MultiplyExpression", required))	{
		return 0;
	}

	if(matchLiteral("*")) {
		consume();

		Node *node = newNode(Node::NodeTypeArith, arg1->line, Node::NodeSubtypeAdd);
		node->children.push_back(arg1);
		node->children.push_back(parseMultiplyExpression(true));

		return node;
	} else {
		return arg1;
	}
}

Node *Parser::parseBaseExpression()
{
	Node *node;

	if(match(Tokenizer::Token::TypeNumber)) {
		node = newNode(Node::NodeTypeConstant, next().line);
		node->lexVal._int = std::atoi(next().text.c_str());
		consume();

		return node;
	} else if(matchLiteral("(")) {
		consume();

		node = parseExpression(true);
		expectLiteral(")");

		return node;
	} else if(node = parseIdentifier()) {
		if(matchLiteral("(")) {
			consume();

			Node *functionCall = newNode(Node::NodeTypeCall, node->line);
			functionCall->children.push_back(node);
			functionCall->children.push_back(parseExpressionList());
			expectLiteral(")");

			node = functionCall;
		}
		return node;
	} else {
		errorExpected("BaseExpression");
		return 0;
	}
}

Node *Parser::parseExpressionList()
{
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
	if(match(Tokenizer::Token::TypeIdentifier))
	{
		Node *node = newNode(Node::NodeTypeId, next().line);
		node->lexVal._id = strdup(mTokenizer.next().text.c_str());
		consume();

		return node;
	}

	return 0;
}

}
