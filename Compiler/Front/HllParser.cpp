#include "Front/HllParser.h"
#include "Front/HllTypeParser.h"

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
Node *HllParser::newNode(Node::NodeType nodeType, int line, Node::NodeSubtype nodeSubtype)
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
 * \brief Check if a string corresponds to a type name
 * \param name Name to check
 * \return true if string is type name, false otherwise
 */
bool HllParser::isTypeName(const std::string &name)
{
	for(unsigned int i=0; i<mTypeNames.size(); i++) {
		if(name == mTypeNames[i]) {
			return true;
		}
	}

	return false;
}

/*!
 * \brief Parse the input stream
 * \return Abstract syntax tree representing parse, or 0 if an error occurred
 */
Node *HllParser::parse()
{
	// Add intrinsic types to type name list
	for(int i=0; i<Types::NumIntrinsics; i++) {
		mTypeNames.push_back(Types::intrinsic((Types::Intrinsic)i)->name);
	}

	// Parse declared type names from program, and add them to the type list
	HllTypeParser typeParser(mHllTokenizer);
	std::set<std::string> types = typeParser.parseTypes();
	mHllTokenizer.reset();
	for(std::set<std::string>::iterator it = types.begin(); it != types.end(); it++) {
		mTypeNames.push_back(*it);
	}

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
	Node *list = newNode(Node::NodeTypeList, next().line);
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

	Node *node = newNode(Node::NodeTypeProcedureDef, returnType->line);
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

	Node *node = newNode(Node::NodeTypeStructDef, next().line);
	consume();

	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

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

Node *HllParser::parseClass()
{
	// <Struct> := 'class' IDENTIFIER { ':' IDENTIFIER }? '{' <ClassMember>* '}'
	if(!matchLiteral("class")) {
		return 0;
	}

	Node *node = newNode(Node::NodeTypeClassDef, next().line);
	consume();

	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	if(matchLiteral(":")) {
		consume();
		std::string parent = next().text;
		expect(HllTokenizer::TypeIdentifier);
		Node *parentNode = newNode(Node::NodeTypeId, next().line);
		parentNode->lexVal.s = parent;
		node->children.push_back(parentNode);
	}

	expectLiteral("{");
	Node *membersNode = newNode(Node::NodeTypeList, next().line);

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
	if(match(HllTokenizer::TypeIdentifier) && isTypeName(next().text) && matchLiteral("(", 1)) {
		type = 0;
	} else {
		type = parseType();
		if(!type) {
			return 0;
		}
	}

	std::string name = next().text;
	expect(HllTokenizer::TypeIdentifier);

	Node *node;
	if(matchLiteral("(")) {
		consume();
		node = newNode(Node::NodeTypeProcedureDef, next().line);
		node->children.push_back(type);
		node->lexVal.s = name;

		node->children.push_back(parseArgumentList());
		expectLiteral(")");

		expectLiteral("{");
		node->children.push_back(parseStatementList());
		expectLiteral("}");
	} else {
		node = newNode(Node::NodeTypeVarDecl, type->line);
		node->lexVal.s = name;
		node->children.push_back(type);
		expectLiteral(";");
	}

	return node;
}

Node *HllParser::parseVariableDeclaration()
{
	// <VariableDeclaration> := <Type> IDENTIFIER
	Node *type = parseType();
	if(!type) return 0;

	Node *node = newNode(Node::NodeTypeVarDecl, type->line);
	node->lexVal.s = next().text;
	expect(HllTokenizer::TypeIdentifier);

	node->children.push_back(type);

	return node;
}

Node *HllParser::parseArgumentList()
{
	// <ArgumentList> := { <VariableDeclaration> ',' }*
	Node *node = newNode(Node::NodeTypeList, next().line);
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

	if(match(HllTokenizer::TypeIdentifier) && isTypeName(next().text)) {
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

	// Throw an error if a type was required and none was found
	if(required) {
		errorExpected("<type>");
	}

	return 0;
}

Node *HllParser::parseStatementList()
{
	// <StatementList> := { <Statement> }*
	Node *node = newNode(Node::NodeTypeList, next().line);

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

		Node *assignNode = newNode(Node::NodeTypeAssign, node->line);
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

Node *HllParser::parseCompareExpression(bool required)
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

Node *HllParser::parseAddExpression(bool required)
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

Node *HllParser::parseMultiplyExpression(bool required)
{
	// <MultiplyExpression> := <SuffixExpression> { '*' <SuffixExpression> }*
	Node *node = parseSuffixExpression(required);
	if(!node) {
		return 0;
	}

	while(true) {
		if(matchLiteral("*") || matchLiteral("/") || matchLiteral("%")) {
			Node::NodeSubtype subtype;
			if(matchLiteral("*")) subtype = Node::NodeSubtypeMultiply;
			else if(matchLiteral("/")) subtype = Node::NodeSubtypeDivide;
			else if(matchLiteral("%")) subtype = Node::NodeSubtypeModulo;
			consume();

			Node *multiplyNode = newNode(Node::NodeTypeArith, node->line, subtype);
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

			Node *callNode = newNode(Node::NodeTypeCall, node->line);
			callNode->children.push_back(node);
			callNode->children.push_back(parseExpressionList());
			expectLiteral(")");

			node = callNode;
		} else if(matchLiteral("[")) {
			// '[' <Expression> ']'
			consume();

			Node *arrayNode = newNode(Node::NodeTypeArray, node->line);
			arrayNode->children.push_back(node);
			arrayNode->children.push_back(parseExpression(true));
			expectLiteral("]");
			node = arrayNode;
		} else if(matchLiteral("++") || matchLiteral("--")) {
			// [ '++' | '--' ]
			Node::NodeSubtype subtype;
			if(matchLiteral("++")) subtype = Node::NodeSubtypeIncrement;
			else if(matchLiteral("--")) subtype = Node::NodeSubtypeDecrement;
			consume();

			Node *incrementNode = newNode(Node::NodeTypeArith, node->line, subtype);
			incrementNode->children.push_back(node);
			node = incrementNode;
		} else if(matchLiteral(".")) {
			Node *memberNode = newNode(Node::NodeTypeMember, next().line);
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
	Node *list = newNode(Node::NodeTypeList, next().line);
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
		node = newNode(Node::NodeTypeConstant, next().line);
		node->lexVal.i = std::atoi(next().text.c_str());
		node->type = Types::intrinsic(Types::Int);
		consume();

		return node;
	} else if(match(HllTokenizer::TypeString)) {
		// <BaseExpression> := STRING
		node = newNode(Node::NodeTypeConstant, next().line);
		node->lexVal.s = next().text;
		node->type = Types::intrinsic(Types::String);
		consume();

		return node;
	} else if(match(HllTokenizer::TypeChar)) {
		// <BaseExpression> := CHAR
		node = newNode(Node::NodeTypeConstant, next().line);
		node->lexVal.i = (int)next().text[0];
		node->type = Types::intrinsic(Types::Char);
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
	} else if(match(HllTokenizer::TypeIdentifier)) {
		// <BaseExpression> := IDENTIFIER
		node = newNode(Node::NodeTypeId, next().line);
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
		node = newNode(Node::NodeTypeNew, next().line);
		consume();

		Node *type = parseType(true);
		Node *args = 0;
		if(matchLiteral("[")) {
			consume();

			Node *count = parseExpression(true);
			expectLiteral("]");
			Node *arrayType = newNode(Node::NodeTypeArray, type->line);
			arrayType->children.push_back(type);
			arrayType->children.push_back(count);
			type = arrayType;
		} else if(matchLiteral("(")) {
			consume();

			args = parseExpressionList();
			expectLiteral(")");
		} else {
			args = newNode(Node::NodeTypeList, node->line);
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
