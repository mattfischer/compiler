#include "Front/Parser.h"

#include "Front/Type.h"

#include <sstream>
#include <cctype>

#include <string.h>

namespace Front {
Parser::Parser(Tokenizer &tokenizer)
	: mTokenizer(tokenizer)
{
	mError = false;
}

const Tokenizer::Token &Parser::next()
{
	return mTokenizer.next();
}

bool Parser::expect(Tokenizer::Token::Type type)
{
	if(mTokenizer.error()) {
		mError = true;
		mErrorMessage = mTokenizer.errorMessage();
		mErrorLine = next().line;
		mErrorColumn = next().column;
		return false;
	}

	if(next().type != type) {
		errorExpected(Tokenizer::Token::typeName(type));
		return false;
	}

	consume();
	return true;
}

bool Parser::expectLiteral(const std::string &text)
{
	if(mTokenizer.error()) {
		mError = true;
		mErrorMessage = mTokenizer.errorMessage();
		mErrorLine = next().line;
		mErrorColumn = next().column;
		return false;
	}

	if(next().type != Tokenizer::Token::TypeLiteral || next().text != text) {
		errorExpected(text);
		return false;
	}

	consume();
	return true;
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
	mError = true;
	std::stringstream ss;
	ss << "expected " << expected << ", '" << next().text << "' found instead";
	mErrorMessage = ss.str();
	mErrorLine = next().line;
	mErrorColumn = next().column;
}

Node *Parser::newNode(Node::NodeType nodeType, Node::NodeSubtype nodeSubtype)
{
	Node *node = new Node;
	node->nodeType = nodeType;
	node->nodeSubtype = nodeSubtype;

	mNodes.push_back(node);

	return node;
}

Node *Parser::parse()
{
	Node *procedureList = parseProcedureList();

	if(error() || !expect(Tokenizer::Token::TypeEnd)) {
		for(unsigned int i=0; i<mNodes.size(); i++) {
			delete mNodes[i];
		}
		return 0;
	}

	return procedureList;
}

Node *Parser::parseProcedureList()
{
	Node *node = newNode(Node::NodeTypeList);
	node->line = 0;

	Node *procedure;
	while(procedure = parseProcedure()) {
		node->children.push_back(procedure);
	}
	if(error()) return 0;

	return node;
}

Node *Parser::parseProcedure()
{
	Node *returnType = parseType();
	if(!returnType)	return 0;
	if(!match(Tokenizer::Token::TypeIdentifier)) return 0;

	std::string name = next().text;
	consume();

	if(!expectLiteral("(")) return 0;
	Node *arguments = parseArgumentDeclarationList();
	if(error()) return 0;
	if(!expectLiteral(")")) return 0;

	if(!expectLiteral("{")) return 0;
	Node *body = parseStatementList();
	if(error()) return 0;
	if(!expectLiteral("}")) return 0;

	Node *node = newNode(Node::NodeTypeProcedureDef);
	node->line = returnType->line;
	node->children.push_back(returnType);
	node->lexVal._id = strdup(name.c_str());
	node->children.push_back(arguments);
	node->children.push_back(body);

	return node;
}

Node *Parser::parseArgumentDeclarationList()
{
	Node *node = newNode(Node::NodeTypeList);
	node->line = 0;

	Node *argument;
	while(argument = parseVariableDeclaration()) {
		node->children.push_back(argument);
		if(matchLiteral(",")) {
			consume();
		} else {
			break;
		}
	}
	if(error()) return 0;

	return node;
}

Node *Parser::parseVariableDeclaration()
{
	Node *type = parseType();
	if(!type) return 0;

	Node *node = newNode(Node::NodeTypeVarDecl);
	node->line = type->line;
	node->children.push_back(type);

	if(!match(Tokenizer::Token::TypeIdentifier)) {
		errorExpected("Identifier");
		return 0;
	}

	std::string name = next().text;
	consume();
	node->lexVal._id = strdup(name.c_str());

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
	Node *node = newNode(Node::NodeTypeList);
	node->line = 0;

	Node *statement;
	while(statement = parseStatement()) {
		node->children.push_back(statement);
	}
	if(error()) return 0;

	return node;
}

Node *Parser::parseStatement()
{
	Node *node;

	if((node = parseVariableDeclaration()) ||
	   (node = parsePrintStatement()) ||
	   (node = parseReturnStatement())) {
		if(!expectLiteral(";")) return 0;

		return node;
	} else if((node = parseIfStatement()) || (node = parseWhileStatement())) {
		return node;
	} else if(match(Tokenizer::Token::TypeIdentifier)) {
		Node *identifier = parseIdentifier();
		if(matchLiteral("=")) {
			node = newNode(Node::NodeTypeAssign);
			node->line = next().line;
			consume();

			node->children.push_back(identifier);

			Node *expression = parseExpression();
			if(error()) return 0;
			if(!expression) {
				errorExpected("Expression");
				return 0;
			}
			node->children.push_back(expression);

			if(!expectLiteral(";")) return 0;

			return node;
		} else if(matchLiteral("(")) {
			node = newNode(Node::NodeTypeCall);
			node->line = next().line;
			consume();

			node->children.push_back(identifier);
			Node *arguments = parseExpressionList();
			if(error()) return 0;
			if(!arguments) {
				errorExpected("ArgumentList");
				return 0;
			}

			node->children.push_back(arguments);

			if(!expectLiteral(")")) return 0;
			if(!expectLiteral(";")) return 0;

			return node;
		} else {
			errorExpected("= or (");
			return 0;
		}
	}

	return 0;
}

Node *Parser::parsePrintStatement()
{
	if(matchLiteral("print")) {
		Node *node = newNode(Node::NodeTypePrint);
		node->line = next().line;
		consume();

		Node *expression = parseExpression();
		if(!expression) {
			errorExpected("Expression");
			return 0;
		}
		node->children.push_back(expression);
		return node;
	}

	return 0;
}

Node *Parser::parseReturnStatement()
{
	if(matchLiteral("return")) {
		Node *node = newNode(Node::NodeTypeReturn);
		node->line = next().line;
		consume();

		Node *expression = parseExpression();
		if(!expression) {
			errorExpected("Expression");
			return 0;
		}
		node->children.push_back(expression);
		return node;
	}

	return 0;
}

Node *Parser::parseIfStatement()
{
	if(matchLiteral("if")) {
		Node *node = newNode(Node::NodeTypeIf);
		node->line = next().line;

		consume();
		if(!expectLiteral("(")) return 0;

		Node *predicate = parseExpression();
		if(error()) return 0;
		if(!predicate) {
			errorExpected("Expression");
			return 0;
		}
		if(!expectLiteral(")")) return 0;
		node->children.push_back(predicate);

		Node *thenClause = parseClause();
		if(error()) return 0;
		if(!thenClause) {
			errorExpected("Clause");
			return 0;
		}
		node->children.push_back(thenClause);

		if(matchLiteral("else")) {
			Node *elseClause = parseClause();
			if(error()) return 0;
			if(!elseClause) {
				errorExpected("Clause");
				return 0;
			}
			node->children.push_back(elseClause);
		}

		return node;
	}

	return 0;
}

Node *Parser::parseWhileStatement()
{
	if(matchLiteral("while")) {
		Node *node = newNode(Node::NodeTypeWhile);
		node->line = next().line;

		consume();
		if(!expectLiteral("(")) return 0;

		Node *predicate = parseExpression();
		if(error()) return 0;
		if(!predicate) {
			errorExpected("Expression");
			return 0;
		}
		if(!expectLiteral(")")) return 0;
		node->children.push_back(predicate);

		Node *clause = parseClause();
		if(error()) return 0;
		if(!clause) {
			errorExpected("Clause");
			return 0;
		}
		node->children.push_back(clause);

		return node;
	}

	return 0;
}

Node *Parser::parseClause()
{
	if(matchLiteral("{")) {
		consume();
		Node *node = parseStatementList();
		if(error()) return 0;
		if(!node) {
			errorExpected("StatementList");
			return 0;
		}
		if(!expectLiteral("}")) return 0;
		return node;
	}

	return parseStatement();
}

Node *Parser::parseExpression()
{
	Node *arg1 = parseAddExpression();
	if(!arg1) return 0;

	if(matchLiteral("==") || matchLiteral("!=")) {
		Node::NodeSubtype subtype;
		if(matchLiteral("==")) subtype = Node::NodeSubtypeEqual;
		else if(matchLiteral("!=")) subtype = Node::NodeSubtypeNequal;
		consume();
		Node *arg2 = parseAddExpression();
		if(!arg2) {
			errorExpected("AddExpression");
			return 0;
		}

		Node *node = newNode(Node::NodeTypeCompare, subtype);
		node->line = arg1->line;
		node->children.push_back(arg1);
		node->children.push_back(arg2);
		return node;
	} else {
		return arg1;
	}
}

Node *Parser::parseAddExpression()
{
	Node *arg1 = parseMultiplyExpression();
	if(!arg1) return 0;

	if(matchLiteral("+")) {
		consume();
		Node *arg2 = parseAddExpression();
		if(!arg2) {
			errorExpected("AddExpression");
			return 0;
		}

		Node *node = newNode(Node::NodeTypeArith, Node::NodeSubtypeAdd);
		node->line = arg1->line;
		node->children.push_back(arg1);
		node->children.push_back(arg2);
		return node;
	} else {
		return arg1;
	}
}

Node *Parser::parseMultiplyExpression()
{
	Node *arg1 = parseBaseExpression();
	if(!arg1) return 0;

	if(matchLiteral("*")) {
		consume();
		Node *arg2 = parseMultiplyExpression();
		if(!arg2) {
			errorExpected("MultiplyExpression");
			return 0;
		}

		Node *node = newNode(Node::NodeTypeArith, Node::NodeSubtypeAdd);
		node->line = arg1->line;
		node->children.push_back(arg1);
		node->children.push_back(arg2);
		return node;
	} else {
		return arg1;
	}
}

Node *Parser::parseBaseExpression()
{
	Node *node;

	if(match(Tokenizer::Token::TypeNumber)) {
		int value = std::atoi(next().text.c_str());
		consume();

		node = newNode(Node::NodeTypeConstant);
		node->lexVal._int = value;
		return node;
	} else if(matchLiteral("(")) {
		consume();
		node = parseExpression();
		if(error()) return 0;
		if(!node) {
			errorExpected("Expression");
			return 0;
		}
		if(!expectLiteral(")")) return 0;

		return node;
	} else if(node = parseIdentifier()) {
		if(matchLiteral("(")) {
			Node *functionCall = newNode(Node::NodeTypeCall);
			functionCall->line = next().line;
			consume();

			functionCall->children.push_back(node);

			Node *arguments = parseExpressionList();
			if(!arguments) {
				errorExpected("ExpressionList");
				return 0;
			}
			functionCall->children.push_back(arguments);

			if(!expectLiteral(")")) return 0;

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
	Node *node = newNode(Node::NodeTypeList);
	Node *expression;
	while(expression = parseExpression()) {
		node->children.push_back(expression);
		if(matchLiteral(",")) {
			consume();
		} else {
			break;
		}
	}
	if(error()) return 0;

	return node;
}

Node *Parser::parseIdentifier()
{
	if(match(Tokenizer::Token::TypeIdentifier))
	{
		Node *node = newNode(Node::NodeTypeId);
		node->line = next().line;
		node->lexVal._id = strdup(mTokenizer.next().text.c_str());
		consume();

		return node;
	}

	return 0;
}

}
