#include "Front/Parser.h"

#include "Front/Types.h"

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
	node->symbol = 0;

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
	for(int i=0; i<Types::NumIntrinsics; i++) {
		mTypeNames.push_back(Types::intrinsic((Types::Intrinsic)i)->name);
	}

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
	// <Program> := { [ <Procedure> | <Struct> ] }* END
	Node *list = newNode(Node::NodeTypeList, next().line);
	while(true) {
		Node *node;
		if(node = parseProcedure()) {
			list->children.push_back(node);
		} else if(node = parseStruct()) {
			list->children.push_back(node);
		} else {
			break;
		}
	}
	expect(Tokenizer::Token::TypeEnd);

	return list;
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
	Node *argumentsNode = newNode(Node::NodeTypeList, next().line);
	Node *argument;
	while(argument = parseVariableDeclaration()) {
		argumentsNode->children.push_back(argument);
		if(!matchLiteral(",")) {
			break;
		}
		consume();
	}

	node->children.push_back(argumentsNode);
	expectLiteral(")");

	expectLiteral("{");
	node->children.push_back(parseStatementList());
	expectLiteral("}");

	return node;
}

Node *Parser::parseStruct()
{
	// <Struct> := 'struct' IDENTIFIER '{' { <VariableDeclaration> ';' }* '}'
	if(!matchLiteral("struct")) {
		return 0;
	}

	Node *node = newNode(Node::NodeTypeStruct, next().line);
	consume();

	node->lexVal.s = next().text;
	expect(Tokenizer::Token::TypeIdentifier);

	expectLiteral("{");
	Node *membersNode = newNode(Node::NodeTypeList, next().line);
	Node *member;
	while(member = parseVariableDeclaration()) {
		membersNode->children.push_back(member);
		expectLiteral(";");
	}
	node->children.push_back(membersNode);
	expectLiteral("}");

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
	// <Type> := IDENTIFIER { '[' ']' }*
	Node *node;

	if(match(Tokenizer::Token::TypeIdentifier)) {
		for(unsigned int i=0; i<mTypeNames.size(); i++) {
			if(next().text == mTypeNames[i]) {
				node = newNode(Node::NodeTypeId, next().line);
				node->lexVal.s = next().text;
				consume();

				while(matchLiteral("[") && matchLiteral("]", 1)) {
					consume(2);
					Node *arrayNode = newNode(Node::NodeTypeArray, node->line);
					arrayNode->children.push_back(node);
					node = arrayNode;
				}
				return node;
			}
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
	} else if(matchLiteral("for")) {
		// <Statement> := 'for' '(' { <VarDecl> '=' }? <Expression> ';' <Expression> ';' <Expression> ')' <Clause>
		node = newNode(Node::NodeTypeFor, next().line);
		consume();

		expectLiteral("(");
		Node *varDecl = parseVariableDeclaration();
		if(varDecl) {
			Node *assign = newNode(Node::NodeTypeAssign, varDecl->line);
			assign->children.push_back(varDecl);

			expectLiteral("=");
			assign->children.push_back(parseExpression(true));
			node->children.push_back(assign);
		} else {
			node->children.push_back(parseExpression(true));
		}
		expectLiteral(";");
		node->children.push_back(parseExpression(true));
		expectLiteral(";");
		node->children.push_back(parseExpression(true));
		expectLiteral(")");

		node->children.push_back(parseClause(true));

		return node;
	} else if(matchLiteral("break")) {
		// 'break' ';'
		node = newNode(Node::NodeTypeBreak, next().line);
		consume();
		expectLiteral(";");

		return node;
	} else if(matchLiteral("continue")) {
		// 'continue' ';'
		node = newNode(Node::NodeTypeContinue, next().line);
		consume();
		expectLiteral(";");

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
	// <Expression> := <OrExpression> { '=' <OrExpression> }*
	Node *node = parseOrExpression(required);
	if(!node) {
		return 0;
	}

	if(matchLiteral("=")) {
		consume();

		Node *assignNode = newNode(Node::NodeTypeAssign, node->line);
		assignNode->children.push_back(node);
		assignNode->children.push_back(parseExpression(true));
		node = assignNode;
	}

	return node;
}

Node *Parser::parseOrExpression(bool required)
{
	// <OrExpression> := <AndExpression> { '||' <AndExpression> }*
	Node *node = parseAndExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("||")) {
			consume();

			Node *orNode = newNode(Node::NodeTypeCompare, node->line, Node::NodeSubtypeOr);
			orNode->children.push_back(node);
			orNode->children.push_back(parseAndExpression(true));
			node = orNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseAndExpression(bool required)
{
	// <AndExpression> := <CompareExpression> { '&&' <CompareExpression> }*
	Node *node = parseCompareExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("&&")) {
			consume();

			Node *andNode = newNode(Node::NodeTypeCompare, node->line, Node::NodeSubtypeAnd);
			andNode->children.push_back(node);
			andNode->children.push_back(parseCompareExpression(true));
			node = andNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseCompareExpression(bool required)
{
	// <CompareExpression> := <AddExpression> { [ '==' | '!=' | '<' | '<=' | '>' | '>=' ] <AddExpression> }*
	Node *node = parseAddExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("==") || matchLiteral("!=") || matchLiteral("<") || matchLiteral(">") || matchLiteral("<=") || matchLiteral(">=")) {
			Node::NodeSubtype subtype;
			if(matchLiteral("==")) subtype = Node::NodeSubtypeEqual;
			else if(matchLiteral("!=")) subtype = Node::NodeSubtypeNequal;
			else if(matchLiteral("<")) subtype = Node::NodeSubtypeLessThan;
			else if(matchLiteral("<=")) subtype = Node::NodeSubtypeLessThanEqual;
			else if(matchLiteral(">")) subtype = Node::NodeSubtypeGreaterThan;
			else if(matchLiteral(">=")) subtype = Node::NodeSubtypeGreaterThanEqual;
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
	// <AddExpression> := <MultiplyExpression> { [ '+' | '-' ] <MultiplyExpression> }*
	Node *node = parseMultiplyExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("+") || matchLiteral("-")) {
			Node::NodeSubtype subtype;
			if(matchLiteral("+")) subtype = Node::NodeSubtypeAdd;
			else if(matchLiteral("-")) subtype = Node::NodeSubtypeSubtract;
			consume();

			Node *addNode = newNode(Node::NodeTypeArith, node->line, subtype);
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
	// <MultiplyExpression> := <SuffixExpression> { '*' <SuffixExpression> }*
	Node *node = parseSuffixExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("*")) {
			consume();

			Node *multiplyNode = newNode(Node::NodeTypeArith, node->line, Node::NodeSubtypeMultiply);
			multiplyNode->children.push_back(node);
			multiplyNode->children.push_back(parseSuffixExpression(true));
			node = multiplyNode;
			continue;
		}
		break;
	}

	return node;
}

Node *Parser::parseSuffixExpression(bool required)
{
	// <SuffixExpression> := <BaseExpression> ...
	Node *node = parseBaseExpression(required);
	if(!node) {
		return 0;
	}

	// { ... }*
	while(true) {
		if(matchLiteral("(")) {
			// '(' { <Expression> ',' }* ')'
			consume();

			Node *callNode = newNode(Node::NodeTypeCall, node->line);
			callNode->children.push_back(node);

			Node *argumentList = newNode(Node::NodeTypeList, next().line);
			Node *expression;
			while(expression = parseExpression()) {
				argumentList->children.push_back(expression);
				if(!matchLiteral(",")) {
					break;
				}
				consume();
			}

			callNode->children.push_back(argumentList);
			expectLiteral(")");
			node = callNode;

			continue;
		} else if(matchLiteral("[")) {
			// '[' <Expression> ']'
			consume();

			Node *arrayNode = newNode(Node::NodeTypeArray, node->line);
			arrayNode->children.push_back(node);
			arrayNode->children.push_back(parseExpression(true));
			expectLiteral("]");
			node = arrayNode;

			continue;
		} else if(matchLiteral("++") || matchLiteral("--")) {
			// [ '++' | '--' ]
			Node::NodeSubtype subtype;
			if(matchLiteral("++")) subtype = Node::NodeSubtypeIncrement;
			else if(matchLiteral("--")) subtype = Node::NodeSubtypeDecrement;
			consume();

			Node *incrementNode = newNode(Node::NodeTypeArith, node->line, subtype);
			incrementNode->children.push_back(node);
			node = incrementNode;

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
		node->type = Types::intrinsic(Types::Int);
		consume();

		return node;
	} else if(matchLiteral("true") || matchLiteral("false")) {
		// <BaseExpression> := 'true' | 'false'
		int value;
		node = newNode(Node::NodeTypeConstant, next().line);
		if(matchLiteral("true")) value = 1;
		else if(matchLiteral("false")) value = 0;
		node->lexVal.i = value;
		node->type = Types::intrinsic(Types::Bool);
		consume();

		return node;
	} else if(match(Tokenizer::Token::TypeIdentifier)) {
		// <BaseExpression> := IDENTIFIER
		node = newNode(Node::NodeTypeId, next().line);
		node->lexVal.s = mTokenizer.next().text;
		consume();

		return node;
	} else if(matchLiteral("(")) {
		// <BaseExpression> := '(' <Expression> ')'
		consume();

		node = parseExpression(true);
		expectLiteral(")");

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

}
