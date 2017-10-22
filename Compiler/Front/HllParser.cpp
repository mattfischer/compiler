#include "Front/HllParser.h"

#include "Front/Types.h"

#include <sstream>
#include <cctype>

namespace Front {

/*!
 * \brief Constructor
 * \param tokenizer Tokenizer
 */
HllParser::HllParser(HllTokenizer &tokenizer)
: Input::Parser(tokenizer),
  mHllTokenizer(tokenizer)
{
}

/*!
 * \brief Construct a new tree node with the given type and line
 * \param nodeType Type for new node
 * \param line Line number to associate with node
 * \param nodeSubtype Subtype for new node
 * \return Newly constructed node
 */
std::unique_ptr<Node> HllParser::newNode(Node::Type nodeType, int line, Node::Subtype nodeSubtype)
{
	// Construct the new node
	std::unique_ptr<Node> node = std::make_unique<Node>();
	node->nodeType = nodeType;
	node->nodeSubtype = nodeSubtype;
	node->type = 0;
	node->line = line;
	node->symbol = 0;

	return node;
}

/*!
 * \brief Parse the input stream
 * \return Abstract syntax tree representing parse, or 0 if an error occurred
 */
std::unique_ptr<Node> HllParser::parse()
{
	try {
		// Parse the stream
		return parseProgram();
	} catch(Input::Parser::ParseException parseException) {
		// Collect the error information from the exception
		setError(parseException.message(), parseException.line(), parseException.column());
		return 0;
	}
}

std::unique_ptr<Node> HllParser::parseProgram()
{
	// <Program> := { [ <Procedure> | <Struct> ] }* END
	std::unique_ptr<Node> list = newNode(Node::Type::List, next().line);
	while(true) {
		std::unique_ptr<Node> node;
		if(node = parseProcedure()) {
			list->children.push_back(std::move(node));
		} else if((node = parseStruct()) || (node = parseClass())) {
			list->children.push_back(std::move(node));
		} else {
			break;
		}
	}
	expect(Input::Tokenizer::Token::TypeEnd);

	return list;
}

std::unique_ptr<Node> HllParser::parseProcedure()
{
	// <Procedure> := <Type> IDENTIFIER '(' <ArgumentDeclarationList> ')' '{' <StatementList> '}'
	std::unique_ptr<Node> returnType = parseType();
	if(!returnType)	return 0;

	std::unique_ptr<Node> node = newNode(Node::Type::ProcedureDef, returnType->line);
	node->children.push_back(std::move(returnType));
	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	expectLiteral("(");
	node->children.push_back(parseArgumentList());
	expectLiteral(")");

	expectLiteral("{");
	node->children.push_back(parseStatementList());
	expectLiteral("}");

	return node;
}

std::unique_ptr<Node> HllParser::parseStruct()
{
	// <Struct> := 'struct' IDENTIFIER '{' { <VariableDeclaration> ';' }* '}'
	if(!matchLiteral("struct")) {
		return 0;
	}

	std::unique_ptr<Node> node = newNode(Node::Type::StructDef, next().line);
	consume();

	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	expectLiteral("{");
	std::unique_ptr<Node> membersNode = newNode(Node::Type::List, next().line);
	std::unique_ptr<Node> member;
	while(member = parseVariableDeclaration()) {
		membersNode->children.push_back(std::move(member));
		expectLiteral(";");
	}
	node->children.push_back(std::move(membersNode));
	expectLiteral("}");

	return node;
}

std::unique_ptr<Node> HllParser::parseClass()
{
	// <Struct> := 'class' IDENTIFIER { ':' IDENTIFIER }? '{' <ClassMember>* '}'
	if(!matchLiteral("class")) {
		return 0;
	}

	std::unique_ptr<Node> node = newNode(Node::Type::ClassDef, next().line);
	consume();

	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	if(matchLiteral(":")) {
		consume();
		std::string parent = next().text;
		expect(HllTokenizer::TypeIdentifier);
		std::unique_ptr<Node> parentNode = newNode(Node::Type::Id, next().line);
		parentNode->lexVal.s = parent;
		node->children.push_back(std::move(parentNode));
	}

	expectLiteral("{");
	std::unique_ptr<Node> membersNode = newNode(Node::Type::List, next().line);

	std::unique_ptr<Node> member;
	while(member = parseClassMember()) {
		membersNode->children.push_back(std::move(member));
	}

	node->children.push_back(std::move(membersNode));
	expectLiteral("}");

	return node;
}

std::unique_ptr<Node> HllParser::parseClassMember()
{
	// <ClassMember> := { <Type> }? IDENTIFIER '(' <ArgumentDeclarationList> ')' '{' <StatementList> '}' | <Type> IDENTIFIER ';'
	std::unique_ptr<Node> type;
	std::vector<std::unique_ptr<Node>> qualifiers;

	if(match(HllTokenizer::TypeIdentifier) && matchLiteral("(", 1)) {
		type = 0;
	} else {
		while(true) {
			if(matchLiteral("virtual")) {
				qualifiers.push_back(newNode(Node::Type::Qualifier, next().line, Node::Subtype::Virtual));
				consume();
			} else if(matchLiteral("native")) {
				qualifiers.push_back(newNode(Node::Type::Qualifier, next().line, Node::Subtype::Native));
				consume();
			} else if(matchLiteral("static")) {
				qualifiers.push_back(newNode(Node::Type::Qualifier, next().line, Node::Subtype::Static));
				consume();
			} else {
				break;
			}
		}

		type = parseType(qualifiers.size() > 0);
		if(!type) {
			return 0;
		}
	}

	std::string name = next().text;
	expect(HllTokenizer::TypeIdentifier);

	std::unique_ptr<Node> node = newNode(Node::Type::ClassMember, next().line);
	std::unique_ptr<Node> qualifiersNode = newNode(Node::Type::List, next().line);
	qualifiersNode->children = std::move(qualifiers);
	node->children.push_back(std::move(qualifiersNode));

	std::unique_ptr<Node> memberNode;
	if(matchLiteral("(")) {
		consume();
		memberNode = newNode(Node::Type::ProcedureDef, next().line);
		memberNode->children.push_back(std::move(type));
		memberNode->lexVal.s = name;

		memberNode->children.push_back(parseArgumentList());
		expectLiteral(")");

		if(matchLiteral("{")) {
			consume();
			memberNode->children.push_back(parseStatementList());
			expectLiteral("}");
		} else {
			expectLiteral(";");
		}
	} else {
		if(qualifiersNode->children.size() > 0) {
			errorExpected("(");
		}

		memberNode = newNode(Node::Type::VarDecl, type->line);
		memberNode->lexVal.s = name;
		memberNode->children.push_back(std::move(type));
		expectLiteral(";");
	}

	node->children.push_back(std::move(memberNode));

	return node;
}

std::unique_ptr<Node> HllParser::parseVariableDeclaration()
{
	// <VariableDeclaration> := <Type> IDENTIFIER
	if(match(HllTokenizer::TypeIdentifier)) {
		if(matchLiteral("[", 1)) {
			if(!matchLiteral("]", 2)) {
				return 0;
			}
		} else if(!match(HllTokenizer::TypeIdentifier, 1)) {
			return 0;
		}
	}

	std::unique_ptr<Node> type = parseType();
	if(!type) return 0;

	std::unique_ptr<Node> node = newNode(Node::Type::VarDecl, type->line);
	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	node->children.push_back(std::move(type));

	return node;
}

std::unique_ptr<Node> HllParser::parseArgumentList()
{
	// <ArgumentList> := { <VariableDeclaration> ',' }*
	std::unique_ptr<Node> node = newNode(Node::Type::List, next().line);
	std::unique_ptr<Node> argument;
	while(argument = parseVariableDeclaration()) {
		node->children.push_back(std::move(argument));
		if(!matchLiteral(",")) {
			break;
		}
		consume();
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseType(bool required)
{
	// <Type> := IDENTIFIER { '[' ']' }*
	std::unique_ptr<Node> node;

	if(match(HllTokenizer::TypeIdentifier)) {
		node = newNode(Node::Type::Id, next().line);
		node->lexVal.s = next().text;
		consume();

		while(matchLiteral("[") && matchLiteral("]", 1)) {
			consume(2);
			std::unique_ptr<Node> arrayNode = newNode(Node::Type::Array, node->line);
			arrayNode->children.push_back(std::move(node));
			node = std::move(arrayNode);
		}
		return node;
	}

	// Throw an error if a type was required and none was found
	if(required) {
		errorExpected("<type>");
	}

	return 0;
}

std::unique_ptr<Node> HllParser::parseStatementList()
{
	// <StatementList> := { <Statement> }*
	std::unique_ptr<Node> node = newNode(Node::Type::List, next().line);

	std::unique_ptr<Node> statement;
	while(statement = parseStatement()) {
		node->children.push_back(std::move(statement));
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseStatement(bool required)
{
	std::unique_ptr<Node> node;

	if(node = parseVariableDeclaration()) {
		// <Statement> := <VariableDeclaration> { = <Expression> ';' }?
		if(matchLiteral("=")) {
			consume();
			std::unique_ptr<Node> assign = newNode(Node::Type::Assign, node->line);
			assign->children.push_back(std::move(node));
			assign->children.push_back(parseExpression(true));
			node = std::move(assign);
		}
		expectLiteral(";");

		return node;
	} else if(node = parseExpression()) {
		// <Statement> := <Expression> ';'
		expectLiteral(";");

		return node;
	} else if(matchLiteral("return")) {
		// <Statement> := 'return' <Expression> ';'
		node = newNode(Node::Type::Return, next().line);
		consume();

		node->children.push_back(parseExpression(true));
		expectLiteral(";");

		return node;
	} else if(matchLiteral("if")) {
		// <Statement> := 'if' '(' <Expression> ')' <Clause> { 'else' <Clause> }?
		node = newNode(Node::Type::If, next().line);
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
		node = newNode(Node::Type::While, next().line);
		consume();

		expectLiteral("(");
		node->children.push_back(parseExpression(true));
		expectLiteral(")");

		node->children.push_back(parseClause(true));

		return node;
	} else if(matchLiteral("for")) {
		// <Statement> := 'for' '(' { <VarDecl> '=' }? <Expression> ';' <Expression> ';' <Expression> ')' <Clause>
		node = newNode(Node::Type::For, next().line);
		consume();

		expectLiteral("(");
		std::unique_ptr<Node> varDecl = parseVariableDeclaration();
		if(varDecl) {
			std::unique_ptr<Node> assign = newNode(Node::Type::Assign, varDecl->line);
			assign->children.push_back(std::move(varDecl));

			expectLiteral("=");
			assign->children.push_back(parseExpression(true));
			node->children.push_back(std::move(assign));
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
		node = newNode(Node::Type::Break, next().line);
		consume();
		expectLiteral(";");

		return node;
	} else if(matchLiteral("continue")) {
		// 'continue' ';'
		node = newNode(Node::Type::Continue, next().line);
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

std::unique_ptr<Node> HllParser::parseClause(bool required)
{
	if(matchLiteral("{")) {
		// <Clause> := '{' <StatementList> '}'
		consume();

		std::unique_ptr<Node> node = parseStatementList();
		expectLiteral("}");

		return node;
	} else {
		// <Clause> := <Statement>
		return parseStatement(required);
	}
}

std::unique_ptr<Node> HllParser::parseExpression(bool required)
{
	// <Expression> := <OrExpression> { '=' <OrExpression> }*
	std::unique_ptr<Node> node = parseOrExpression(required);
	if(!node) {
		return 0;
	}

	if(matchLiteral("=")) {
		consume();

		std::unique_ptr<Node> assignNode = newNode(Node::Type::Assign, node->line);
		assignNode->children.push_back(std::move(node));
		assignNode->children.push_back(parseExpression(true));
		node = std::move(assignNode);
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseOrExpression(bool required)
{
	// <OrExpression> := <AndExpression> { '||' <AndExpression> }*
	std::unique_ptr<Node> node = parseAndExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("||")) {
			consume();

			std::unique_ptr<Node> orNode = newNode(Node::Type::Compare, node->line, Node::Subtype::Or);
			orNode->children.push_back(std::move(node));
			orNode->children.push_back(parseAndExpression(true));
			node = std::move(orNode);
			continue;
		}
		break;
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseAndExpression(bool required)
{
	// <AndExpression> := <CompareExpression> { '&&' <CompareExpression> }*
	std::unique_ptr<Node> node = parseCompareExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("&&")) {
			consume();

			std::unique_ptr<Node> andNode = newNode(Node::Type::Compare, node->line, Node::Subtype::And);
			andNode->children.push_back(std::move(node));
			andNode->children.push_back(parseCompareExpression(true));
			node = std::move(andNode);
			continue;
		}
		break;
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseCompareExpression(bool required)
{
	// <CompareExpression> := <AddExpression> { [ '==' | '!=' | '<' | '<=' | '>' | '>=' ] <AddExpression> }*
	std::unique_ptr<Node> node = parseAddExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("==") || matchLiteral("!=") || matchLiteral("<") || matchLiteral(">") || matchLiteral("<=") || matchLiteral(">=")) {
			Node::Subtype subtype;
			if(matchLiteral("==")) subtype = Node::Subtype::Equal;
			else if(matchLiteral("!=")) subtype = Node::Subtype::Nequal;
			else if(matchLiteral("<")) subtype = Node::Subtype::LessThan;
			else if(matchLiteral("<=")) subtype = Node::Subtype::LessThanEqual;
			else if(matchLiteral(">")) subtype = Node::Subtype::GreaterThan;
			else if(matchLiteral(">=")) subtype = Node::Subtype::GreaterThanEqual;
			consume();

			std::unique_ptr<Node> compareNode = newNode(Node::Type::Compare, node->line, subtype);
			compareNode->children.push_back(std::move(node));
			compareNode->children.push_back(parseAddExpression(true));
			node = std::move(compareNode);
			continue;
		}
		break;
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseAddExpression(bool required)
{
	// <AddExpression> := <MultiplyExpression> { [ '+' | '-' ] <MultiplyExpression> }*
	std::unique_ptr<Node> node = parseMultiplyExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("+") || matchLiteral("-")) {
			Node::Subtype subtype;
			if(matchLiteral("+")) subtype = Node::Subtype::Add;
			else if(matchLiteral("-")) subtype = Node::Subtype::Subtract;
			consume();

			std::unique_ptr<Node> addNode = newNode(Node::Type::Arith, node->line, subtype);
			addNode->children.push_back(std::move(node));
			addNode->children.push_back(parseMultiplyExpression(true));
			node = std::move(addNode);
			continue;
		}
		break;
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseMultiplyExpression(bool required)
{
	// <MultiplyExpression> := <SuffixExpression> { '*' <SuffixExpression> }*
	std::unique_ptr<Node> node = parseSuffixExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("*") || matchLiteral("/") || matchLiteral("%")) {
			Node::Subtype subtype;
			if(matchLiteral("*")) subtype = Node::Subtype::Multiply;
			else if(matchLiteral("/")) subtype = Node::Subtype::Divide;
			else if(matchLiteral("%")) subtype = Node::Subtype::Modulo;
			consume();

			std::unique_ptr<Node> multiplyNode = newNode(Node::Type::Arith, node->line, subtype);
			multiplyNode->children.push_back(std::move(node));
			multiplyNode->children.push_back(parseSuffixExpression(true));
			node = std::move(multiplyNode);
			continue;
		}
		break;
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseSuffixExpression(bool required)
{
	// <SuffixExpression> := <BaseExpression> ...
	std::unique_ptr<Node> node = parseBaseExpression(required);
	if(!node) {
		return 0;
	}

	// { ... }*
	while(true) {
		if(matchLiteral("(")) {
			// '(' <ExpressionList> ')'
			consume();

			std::unique_ptr<Node> callNode = newNode(Node::Type::Call, node->line);
			callNode->children.push_back(std::move(node));
			callNode->children.push_back(parseExpressionList());
			expectLiteral(")");

			node = std::move(callNode);
		} else if(matchLiteral("[")) {
			// '[' <Expression> ']'
			consume();

			std::unique_ptr<Node> arrayNode = newNode(Node::Type::Array, node->line);
			arrayNode->children.push_back(std::move(node));
			arrayNode->children.push_back(parseExpression(true));
			expectLiteral("]");
			node = std::move(arrayNode);
		} else if(matchLiteral("++") || matchLiteral("--")) {
			// [ '++' | '--' ]
			Node::Subtype subtype;
			if(matchLiteral("++")) subtype = Node::Subtype::Increment;
			else if(matchLiteral("--")) subtype = Node::Subtype::Decrement;
			consume();

			std::unique_ptr<Node> incrementNode = newNode(Node::Type::Arith, node->line, subtype);
			incrementNode->children.push_back(std::move(node));
			node = std::move(incrementNode);
		} else if(matchLiteral(".")) {
			std::unique_ptr<Node> memberNode = newNode(Node::Type::Member, next().line);
			consume();

			memberNode->lexVal.s = next().text;
			expect(HllTokenizer::TypeIdentifier);
			memberNode->children.push_back(std::move(node));
			node = std::move(memberNode);
		} else {
			break;
		}
	}

	return node;
}

std::unique_ptr<Node> HllParser::parseExpressionList()
{
	// <ExpressionList> := { <Expression> ',' }*
	std::unique_ptr<Node> list = newNode(Node::Type::List, next().line);
	std::unique_ptr<Node> expression;
	while(expression = parseExpression()) {
		list->children.push_back(std::move(expression));
		if(!matchLiteral(",")) {
			break;
		}
		consume();
	}

	return list;
}

std::unique_ptr<Node> HllParser::parseBaseExpression(bool required)
{
	std::unique_ptr<Node> node;

	if(match(HllTokenizer::TypeNumber)) {
		// <BaseExpression> := NUMBER
		node = newNode(Node::Type::Constant, next().line);
		node->lexVal.i = std::atoi(next().text.c_str());
		node->type = Types::intrinsic(Types::Int);
		consume();

		return node;
	} else if(match(HllTokenizer::TypeString)) {
		// <BaseExpression> := STRING
		node = newNode(Node::Type::Constant, next().line);
		node->lexVal.s = next().text;
		node->type = Types::intrinsic(Types::String);
		consume();

		return node;
	} else if(match(HllTokenizer::TypeChar)) {
		// <BaseExpression> := CHAR
		node = newNode(Node::Type::Constant, next().line);
		node->lexVal.i = (int)next().text[0];
		node->type = Types::intrinsic(Types::Char);
		consume();

		return node;
	} else if(matchLiteral("true") || matchLiteral("false")) {
		// <BaseExpression> := 'true' | 'false'
		int value;
		node = newNode(Node::Type::Constant, next().line);
		if(matchLiteral("true")) value = 1;
		else if(matchLiteral("false")) value = 0;
		node->lexVal.i = value;
		node->type = Types::intrinsic(Types::Bool);
		consume();

		return node;
	} else if(match(HllTokenizer::TypeIdentifier)) {
		// <BaseExpression> := IDENTIFIER
		node = newNode(Node::Type::Id, next().line);
		node->lexVal.s = next().text;
		consume();

		return node;
	} else if(matchLiteral("(")) {
		// <BaseExpression> := '(' <Expression> ')'
		consume();

		node = parseExpression(true);
		expectLiteral(")");

		return node;
	} else if(matchLiteral("new")) {
		// <BaseExpression> := 'new' <Type> { '[' <Expression> ']' | '(' <ExpressionList> ')' }?
		node = newNode(Node::Type::New, next().line);
		consume();

		std::unique_ptr<Node> type = parseType(true);
		std::unique_ptr<Node> args;
		if(matchLiteral("[")) {
			consume();

			std::unique_ptr<Node> count = parseExpression(true);
			expectLiteral("]");
			std::unique_ptr<Node> arrayType = newNode(Node::Type::Array, type->line);
			arrayType->children.push_back(std::move(type));
			arrayType->children.push_back(std::move(count));
			type = std::move(arrayType);
		} else if(matchLiteral("(")) {
			consume();

			args = parseExpressionList();
			expectLiteral(")");
		} else {
			args = newNode(Node::Type::List, node->line);
		}

		node->children.push_back(std::move(type));
		if(args) {
			node->children.push_back(std::move(args));
		}

		return node;
	}

	// Throw an error if a base expression was required and none was found
	if(required) {
		errorExpected("<base-expression>");
	}

	return 0;
}

}
