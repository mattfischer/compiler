#include "Front/HandParser.h"

#include "Front/Type.h"

#include <sstream>
#include <cctype>

#include <string.h>

namespace Front {
HandParser::HandParser(Tokenizer &tokenizer)
	: mTokenizer(tokenizer)
{
	mError = false;
}

const Tokenizer::Token &HandParser::next()
{
	return mTokenizer.next();
}

bool HandParser::expect(Tokenizer::Token::Type type)
{
	if(mTokenizer.error()) {
		mError = true;
		mErrorMessage = mTokenizer.errorMessage();
		return false;
	}

	if(next().type != type) {
		errorExpected(Tokenizer::Token::typeName(type));
		return false;
	}

	consume();
	return true;
}

bool HandParser::expectLiteral(const std::string &text)
{
	if(mTokenizer.error()) {
		mError = true;
		mErrorMessage = mTokenizer.errorMessage();
		return false;
	}

	if(next().type != Tokenizer::Token::TypeLiteral || next().text != text) {
		errorExpected(text);
		return false;
	}

	consume();
	return true;
}

bool HandParser::match(Tokenizer::Token::Type type)
{
	if(mTokenizer.error() || next().type != type) {
		return false;
	}

	return true;
}

bool HandParser::matchLiteral(const std::string &text)
{
	if(mTokenizer.error() || next().type != Tokenizer::Token::TypeLiteral || next().text != text) {
		return false;
	}

	return true;
}

void HandParser::consume()
{
	mTokenizer.consume();
}

void HandParser::errorExpected(const std::string &expected)
{
	mError = true;
	std::stringstream ss;
	ss << "expected " << expected << ", '" << next().text << "' found instead";
	mErrorMessage = ss.str();
}

SyntaxNode *HandParser::newNode(SyntaxNode::NodeType nodeType, SyntaxNode::NodeSubtype nodeSubtype)
{
	SyntaxNode *node = new SyntaxNode;
	node->nodeType = nodeType;
	node->nodeSubtype = nodeSubtype;

	mNodes.push_back(node);

	return node;
}

SyntaxNode *HandParser::parse()
{
	SyntaxNode *procedureList = parseProcedureList();

	if(!procedureList || !expect(Tokenizer::Token::TypeEnd)) {
		for(unsigned int i=0; i<mNodes.size(); i++) {
			delete mNodes[i];
		}
		return 0;
	}

	return procedureList;
}

SyntaxNode *HandParser::parseProcedureList()
{
	SyntaxNode *node = newNode(SyntaxNode::NodeTypeList);
	node->line = 0;

	SyntaxNode *procedure;
	while(procedure = parseProcedure()) {
		node->children.push_back(procedure);
	}
	if(error()) return 0;

	return node;
}

SyntaxNode *HandParser::parseProcedure()
{
	SyntaxNode *returnType = parseType();
	if(!returnType)	return 0;
	if(!match(Tokenizer::Token::TypeIdentifier)) return 0;

	std::string name = next().text;
	consume();

	if(!expectLiteral("(")) return 0;
	SyntaxNode *arguments = parseArgumentDeclarationList();
	if(error()) return 0;
	if(!expectLiteral(")")) return 0;

	if(!expectLiteral("{")) return 0;
	SyntaxNode *body = parseStatementList();
	if(error()) return 0;
	if(!expectLiteral("}")) return 0;

	SyntaxNode *node = newNode(SyntaxNode::NodeTypeProcedureDef);
	node->line = returnType->line;
	node->children.push_back(returnType);
	node->lexVal._id = strdup(name.c_str());
	node->children.push_back(arguments);
	node->children.push_back(body);

	return node;
}

SyntaxNode *HandParser::parseArgumentDeclarationList()
{
	SyntaxNode *node = newNode(SyntaxNode::NodeTypeList);
	node->line = 0;

	SyntaxNode *argument;
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

SyntaxNode *HandParser::parseVariableDeclaration()
{
	SyntaxNode *type = parseType();
	if(!type) return 0;

	SyntaxNode *node = newNode(SyntaxNode::NodeTypeVarDecl);
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

SyntaxNode *HandParser::parseType()
{
	if(match(Tokenizer::Token::TypeIdentifier)) {
		if(Type::find(next().text)) {
			return parseIdentifier();
		}
	}

	return 0;
}

SyntaxNode *HandParser::parseStatementList()
{
	SyntaxNode *node = newNode(SyntaxNode::NodeTypeList);
	node->line = 0;

	SyntaxNode *statement;
	while(statement = parseStatement()) {
		node->children.push_back(statement);
	}
	if(error()) return 0;

	return node;
}

SyntaxNode *HandParser::parseStatement()
{
	SyntaxNode *node;

	if((node = parseVariableDeclaration()) ||
	   (node = parsePrintStatement()) ||
	   (node = parseReturnStatement())) {
		if(!expectLiteral(";")) return 0;

		return node;
	} else if((node = parseIfStatement()) || (node = parseWhileStatement())) {
		return node;
	} else if(match(Tokenizer::Token::TypeIdentifier)) {
		SyntaxNode *identifier = parseIdentifier();
		if(matchLiteral("=")) {
			node = newNode(SyntaxNode::NodeTypeAssign);
			node->line = next().line;
			consume();

			node->children.push_back(identifier);

			SyntaxNode *expression = parseExpression();
			if(error()) return 0;
			if(!expression) {
				errorExpected("Expression");
				return 0;
			}
			node->children.push_back(expression);

			if(!expectLiteral(";")) return 0;

			return node;
		} else if(matchLiteral("(")) {
			node = newNode(SyntaxNode::NodeTypeCall);
			node->line = next().line;
			consume();

			node->children.push_back(identifier);
			SyntaxNode *arguments = parseExpressionList();
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

SyntaxNode *HandParser::parsePrintStatement()
{
	if(matchLiteral("print")) {
		SyntaxNode *node = newNode(SyntaxNode::NodeTypePrint);
		node->line = next().line;
		consume();

		SyntaxNode *expression = parseExpression();
		if(!expression) {
			errorExpected("Expression");
			return 0;
		}
		node->children.push_back(expression);
		return node;
	}

	return 0;
}

SyntaxNode *HandParser::parseReturnStatement()
{
	if(matchLiteral("return")) {
		SyntaxNode *node = newNode(SyntaxNode::NodeTypeReturn);
		node->line = next().line;
		consume();

		SyntaxNode *expression = parseExpression();
		if(!expression) {
			errorExpected("Expression");
			return 0;
		}
		node->children.push_back(expression);
		return node;
	}

	return 0;
}

SyntaxNode *HandParser::parseIfStatement()
{
	if(matchLiteral("if")) {
		SyntaxNode *node = newNode(SyntaxNode::NodeTypeIf);
		node->line = next().line;

		consume();
		if(!expectLiteral("(")) return 0;

		SyntaxNode *predicate = parseExpression();
		if(error()) return 0;
		if(!predicate) {
			errorExpected("Expression");
			return 0;
		}
		if(!expectLiteral(")")) return 0;
		node->children.push_back(predicate);

		SyntaxNode *thenClause = parseClause();
		if(error()) return 0;
		if(!thenClause) {
			errorExpected("Clause");
			return 0;
		}
		node->children.push_back(thenClause);

		if(matchLiteral("else")) {
			SyntaxNode *elseClause = parseClause();
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

SyntaxNode *HandParser::parseWhileStatement()
{
	if(matchLiteral("while")) {
		SyntaxNode *node = newNode(SyntaxNode::NodeTypeWhile);
		node->line = next().line;

		consume();
		if(!expectLiteral("(")) return 0;

		SyntaxNode *predicate = parseExpression();
		if(error()) return 0;
		if(!predicate) {
			errorExpected("Expression");
			return 0;
		}
		if(!expectLiteral(")")) return 0;
		node->children.push_back(predicate);

		SyntaxNode *clause = parseClause();
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

SyntaxNode *HandParser::parseClause()
{
	if(matchLiteral("{")) {
		consume();
		SyntaxNode *node = parseStatementList();
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

SyntaxNode *HandParser::parseExpression()
{
	SyntaxNode *arg1 = parseAddExpression();
	if(!arg1) return 0;

	if(matchLiteral("==") || matchLiteral("!=")) {
		SyntaxNode::NodeSubtype subtype;
		if(matchLiteral("==")) subtype = SyntaxNode::NodeSubtypeEqual;
		else if(matchLiteral("!=")) subtype = SyntaxNode::NodeSubtypeNequal;
		consume();
		SyntaxNode *arg2 = parseAddExpression();
		if(!arg2) {
			errorExpected("AddExpression");
			return 0;
		}

		SyntaxNode *node = newNode(SyntaxNode::NodeTypeCompare, subtype);
		node->line = arg1->line;
		node->children.push_back(arg1);
		node->children.push_back(arg2);
		return node;
	} else {
		return arg1;
	}
}

SyntaxNode *HandParser::parseAddExpression()
{
	SyntaxNode *arg1 = parseMultiplyExpression();
	if(!arg1) return 0;

	if(matchLiteral("+")) {
		consume();
		SyntaxNode *arg2 = parseAddExpression();
		if(!arg2) {
			errorExpected("AddExpression");
			return 0;
		}

		SyntaxNode *node = newNode(SyntaxNode::NodeTypeArith, SyntaxNode::NodeSubtypeAdd);
		node->line = arg1->line;
		node->children.push_back(arg1);
		node->children.push_back(arg2);
		return node;
	} else {
		return arg1;
	}
}

SyntaxNode *HandParser::parseMultiplyExpression()
{
	SyntaxNode *arg1 = parseBaseExpression();
	if(!arg1) return 0;

	if(matchLiteral("*")) {
		consume();
		SyntaxNode *arg2 = parseMultiplyExpression();
		if(!arg2) {
			errorExpected("MultiplyExpression");
			return 0;
		}

		SyntaxNode *node = newNode(SyntaxNode::NodeTypeArith, SyntaxNode::NodeSubtypeAdd);
		node->line = arg1->line;
		node->children.push_back(arg1);
		node->children.push_back(arg2);
		return node;
	} else {
		return arg1;
	}
}

SyntaxNode *HandParser::parseBaseExpression()
{
	SyntaxNode *node;

	if(match(Tokenizer::Token::TypeNumber)) {
		int value = std::atoi(next().text.c_str());
		consume();

		node = newNode(SyntaxNode::NodeTypeConstant);
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
			SyntaxNode *functionCall = newNode(SyntaxNode::NodeTypeCall);
			functionCall->line = next().line;
			consume();

			functionCall->children.push_back(node);

			SyntaxNode *arguments = parseExpressionList();
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

SyntaxNode *HandParser::parseExpressionList()
{
	SyntaxNode *node = newNode(SyntaxNode::NodeTypeList);
	SyntaxNode *expression;
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

SyntaxNode *HandParser::parseIdentifier()
{
	if(match(Tokenizer::Token::TypeIdentifier))
	{
		SyntaxNode *node = newNode(SyntaxNode::NodeTypeId);
		node->line = next().line;
		node->lexVal._id = strdup(mTokenizer.next().text.c_str());
		consume();

		return node;
	}

	return 0;
}

}
