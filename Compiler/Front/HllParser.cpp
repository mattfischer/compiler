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
Node *HllParser::newNode(Node::Type nodeType, int line, Node::Subtype nodeSubtype)
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
Node *HllParser::parse()
{
	try {
		// Parse the stream
		return parseProgram();
	} catch(Input::Parser::ParseException parseException) {
		// Collect the error information from the exception
		setError(parseException.message(), parseException.line(), parseException.column());

		// Delete all nodes created along the way
		for(unsigned int i=0; i<mNodes.size(); i++) {
			delete mNodes[i];
		}
		return 0;
	}
}

Node *HllParser::parseProgram()
{
	// <Program> := { [ <Procedure> | <Struct> ] }* END
	Node *list = newNode(Node::Type::List, next().line);
	while(true) {
		Node *node;
		if(node = parseProcedure()) {
			list->children.push_back(node);
		} else if((node = parseStruct()) || (node = parseClass())) {
			list->children.push_back(node);
		} else {
			break;
		}
	}
	expect(Input::Tokenizer::Token::TypeEnd);

	return list;
}

Node *HllParser::parseProcedure()
{
	// <Procedure> := <Type> IDENTIFIER '(' <ArgumentDeclarationList> ')' '{' <StatementList> '}'
	Node *returnType = parseType();
	if(!returnType)	return 0;

	Node *node = newNode(Node::Type::ProcedureDef, returnType->line);
	node->children.push_back(returnType);
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

Node *HllParser::parseStruct()
{
	// <Struct> := 'struct' IDENTIFIER '{' { <VariableDeclaration> ';' }* '}'
	if(!matchLiteral("struct")) {
		return 0;
	}

	Node *node = newNode(Node::Type::StructDef, next().line);
	consume();

	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	expectLiteral("{");
	Node *membersNode = newNode(Node::Type::List, next().line);
	Node *member;
	while(member = parseVariableDeclaration()) {
		membersNode->children.push_back(member);
		expectLiteral(";");
	}
	node->children.push_back(membersNode);
	expectLiteral("}");

	return node;
}

Node *HllParser::parseClass()
{
	// <Struct> := 'class' IDENTIFIER { ':' IDENTIFIER }? '{' <ClassMember>* '}'
	if(!matchLiteral("class")) {
		return 0;
	}

	Node *node = newNode(Node::Type::ClassDef, next().line);
	consume();

	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	if(matchLiteral(":")) {
		consume();
		std::string parent = next().text;
		expect(HllTokenizer::TypeIdentifier);
		Node *parentNode = newNode(Node::Type::Id, next().line);
		parentNode->lexVal.s = parent;
		node->children.push_back(parentNode);
	}

	expectLiteral("{");
	Node *membersNode = newNode(Node::Type::List, next().line);

	Node *member;
	while(member = parseClassMember()) {
		membersNode->children.push_back(member);
	}

	node->children.push_back(membersNode);
	expectLiteral("}");

	return node;
}

Node *HllParser::parseClassMember()
{
	// <ClassMember> := { <Type> }? IDENTIFIER '(' <ArgumentDeclarationList> ')' '{' <StatementList> '}' | <Type> IDENTIFIER ';'
	Node *type;
	std::vector<Node*> qualifiers;

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

	Node *node = newNode(Node::Type::ClassMember, next().line);
	Node *qualifiersNode = newNode(Node::Type::List, next().line);
	qualifiersNode->children = qualifiers;
	node->children.push_back(qualifiersNode);

	Node *memberNode;
	if(matchLiteral("(")) {
		consume();
		memberNode = newNode(Node::Type::ProcedureDef, next().line);
		memberNode->children.push_back(type);
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
		memberNode->children.push_back(type);
		expectLiteral(";");
	}

	node->children.push_back(memberNode);

	return node;
}

Node *HllParser::parseVariableDeclaration()
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

	Node *type = parseType();
	if(!type) return 0;

	Node *node = newNode(Node::Type::VarDecl, type->line);
	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	node->children.push_back(type);

	return node;
}

Node *HllParser::parseArgumentList()
{
	// <ArgumentList> := { <VariableDeclaration> ',' }*
	Node *node = newNode(Node::Type::List, next().line);
	Node *argument;
	while(argument = parseVariableDeclaration()) {
		node->children.push_back(argument);
		if(!matchLiteral(",")) {
			break;
		}
		consume();
	}

	return node;
}

Node *HllParser::parseType(bool required)
{
	// <Type> := IDENTIFIER { '[' ']' }*
	Node *node;

	if(match(HllTokenizer::TypeIdentifier)) {
		node = newNode(Node::Type::Id, next().line);
		node->lexVal.s = next().text;
		consume();

		while(matchLiteral("[") && matchLiteral("]", 1)) {
			consume(2);
			Node *arrayNode = newNode(Node::Type::Array, node->line);
			arrayNode->children.push_back(node);
			node = arrayNode;
		}
		return node;
	}

	// Throw an error if a type was required and none was found
	if(required) {
		errorExpected("<type>");
	}

	return 0;
}

Node *HllParser::parseStatementList()
{
	// <StatementList> := { <Statement> }*
	Node *node = newNode(Node::Type::List, next().line);

	Node *statement;
	while(statement = parseStatement()) {
		node->children.push_back(statement);
	}

	return node;
}

Node *HllParser::parseStatement(bool required)
{
	Node *node;

	if(node = parseVariableDeclaration()) {
		// <Statement> := <VariableDeclaration> { = <Expression> ';' }?
		if(matchLiteral("=")) {
			consume();
			Node *assign = newNode(Node::Type::Assign, node->line);
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
		Node *varDecl = parseVariableDeclaration();
		if(varDecl) {
			Node *assign = newNode(Node::Type::Assign, varDecl->line);
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

Node *HllParser::parseClause(bool required)
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

Node *HllParser::parseExpression(bool required)
{
	// <Expression> := <OrExpression> { '=' <OrExpression> }*
	Node *node = parseOrExpression(required);
	if(!node) {
		return 0;
	}

	if(matchLiteral("=")) {
		consume();

		Node *assignNode = newNode(Node::Type::Assign, node->line);
		assignNode->children.push_back(node);
		assignNode->children.push_back(parseExpression(true));
		node = assignNode;
	}

	return node;
}

Node *HllParser::parseOrExpression(bool required)
{
	// <OrExpression> := <AndExpression> { '||' <AndExpression> }*
	Node *node = parseAndExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("||")) {
			consume();

			Node *orNode = newNode(Node::Type::Compare, node->line, Node::Subtype::Or);
			orNode->children.push_back(node);
			orNode->children.push_back(parseAndExpression(true));
			node = orNode;
			continue;
		}
		break;
	}

	return node;
}

Node *HllParser::parseAndExpression(bool required)
{
	// <AndExpression> := <CompareExpression> { '&&' <CompareExpression> }*
	Node *node = parseCompareExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("&&")) {
			consume();

			Node *andNode = newNode(Node::Type::Compare, node->line, Node::Subtype::And);
			andNode->children.push_back(node);
			andNode->children.push_back(parseCompareExpression(true));
			node = andNode;
			continue;
		}
		break;
	}

	return node;
}

Node *HllParser::parseCompareExpression(bool required)
{
	// <CompareExpression> := <AddExpression> { [ '==' | '!=' | '<' | '<=' | '>' | '>=' ] <AddExpression> }*
	Node *node = parseAddExpression(required);
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

			Node *compareNode = newNode(Node::Type::Compare, node->line, subtype);
			compareNode->children.push_back(node);
			compareNode->children.push_back(parseAddExpression(true));
			node = compareNode;
			continue;
		}
		break;
	}

	return node;
}

Node *HllParser::parseAddExpression(bool required)
{
	// <AddExpression> := <MultiplyExpression> { [ '+' | '-' ] <MultiplyExpression> }*
	Node *node = parseMultiplyExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("+") || matchLiteral("-")) {
			Node::Subtype subtype;
			if(matchLiteral("+")) subtype = Node::Subtype::Add;
			else if(matchLiteral("-")) subtype = Node::Subtype::Subtract;
			consume();

			Node *addNode = newNode(Node::Type::Arith, node->line, subtype);
			addNode->children.push_back(node);
			addNode->children.push_back(parseMultiplyExpression(true));
			node = addNode;
			continue;
		}
		break;
	}

	return node;
}

Node *HllParser::parseMultiplyExpression(bool required)
{
	// <MultiplyExpression> := <SuffixExpression> { '*' <SuffixExpression> }*
	Node *node = parseSuffixExpression(required);
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

			Node *multiplyNode = newNode(Node::Type::Arith, node->line, subtype);
			multiplyNode->children.push_back(node);
			multiplyNode->children.push_back(parseSuffixExpression(true));
			node = multiplyNode;
			continue;
		}
		break;
	}

	return node;
}

Node *HllParser::parseSuffixExpression(bool required)
{
	// <SuffixExpression> := <BaseExpression> ...
	Node *node = parseBaseExpression(required);
	if(!node) {
		return 0;
	}

	// { ... }*
	while(true) {
		if(matchLiteral("(")) {
			// '(' <ExpressionList> ')'
			consume();

			Node *callNode = newNode(Node::Type::Call, node->line);
			callNode->children.push_back(node);
			callNode->children.push_back(parseExpressionList());
			expectLiteral(")");

			node = callNode;
		} else if(matchLiteral("[")) {
			// '[' <Expression> ']'
			consume();

			Node *arrayNode = newNode(Node::Type::Array, node->line);
			arrayNode->children.push_back(node);
			arrayNode->children.push_back(parseExpression(true));
			expectLiteral("]");
			node = arrayNode;
		} else if(matchLiteral("++") || matchLiteral("--")) {
			// [ '++' | '--' ]
			Node::Subtype subtype;
			if(matchLiteral("++")) subtype = Node::Subtype::Increment;
			else if(matchLiteral("--")) subtype = Node::Subtype::Decrement;
			consume();

			Node *incrementNode = newNode(Node::Type::Arith, node->line, subtype);
			incrementNode->children.push_back(node);
			node = incrementNode;
		} else if(matchLiteral(".")) {
			Node *memberNode = newNode(Node::Type::Member, next().line);
			consume();

			memberNode->lexVal.s = next().text;
			expect(HllTokenizer::TypeIdentifier);
			memberNode->children.push_back(node);
			node = memberNode;
		} else {
			break;
		}
	}

	return node;
}

Node *HllParser::parseExpressionList()
{
	// <ExpressionList> := { <Expression> ',' }*
	Node *list = newNode(Node::Type::List, next().line);
	Node *expression;
	while(expression = parseExpression()) {
		list->children.push_back(expression);
		if(!matchLiteral(",")) {
			break;
		}
		consume();
	}

	return list;
}

Node *HllParser::parseBaseExpression(bool required)
{
	Node *node;

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

		Node *type = parseType(true);
		Node *args = 0;
		if(matchLiteral("[")) {
			consume();

			Node *count = parseExpression(true);
			expectLiteral("]");
			Node *arrayType = newNode(Node::Type::Array, type->line);
			arrayType->children.push_back(type);
			arrayType->children.push_back(count);
			type = arrayType;
		} else if(matchLiteral("(")) {
			consume();

			args = parseExpressionList();
			expectLiteral(")");
		} else {
			args = newNode(Node::Type::List, node->line);
		}

		node->children.push_back(type);
		if(args) {
			node->children.push_back(args);
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
