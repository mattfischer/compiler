%{
	#include <stdio.h>
	#include <stdarg.h>
	
	#include "ParseNode.h"
	
	static ParseNode *tree;
	
	ParseNode *addChild(ParseNode *node, ParseNode *child);
	ParseNode *newNode(ParseNodeType type, ...);
%}

%union {
	ParseNode *_node;
	int _int;
	char *_id;
};

%token PRINT
%token <_int> INT
%token <_id> ID
%token NEWL
%token END
%start s

%type <_node> statementlist statement expression term factor id
%%
s: statementlist END
	{ tree = $1; }
 | statementlist NEWL END
	{ tree = $1; }

statementlist:	statementlist NEWL statement
	{ $$ = addChild($1, $3); }
			  | statement
	{ $$ = newNode(ParseNodeStatementList, $1, NULL);	}

statement: PRINT expression
	{ $$ = newNode(ParseNodePrint, $2, NULL); }
	     | id id
	{ $$ = newNode(ParseNodeVarDecl, $1, $2, NULL); }

expression: expression '+' term
	{ $$ = newNode(ParseNodeAdd, $1, $3, NULL); }
		  | term
	{ $$ = $1; }
	
term: term '*' factor
	{ $$ = newNode(ParseNodeMultiply, $1, $3, NULL); }
	| factor
	{ $$ = $1; }
	
factor: INT
	{ $$ = newNode(ParseNodeInt, NULL); $$->lexVal._int = $1; }
	  | id
	{ $$ = $1; }
	  | '(' expression ')'
	{ $$ = $2; }

id: ID
	{ $$ = newNode(ParseNodeId, NULL); $$->lexVal._id = $1; }

%%

extern FILE *langin;

ParseNode *parseLang(const char *filename)
{
	fopen_s(&langin, filename, "r");
	langparse();
	
	return tree;
}

int langerror(char const *s)
{
	return 0;
}

ParseNode *newNode(ParseNodeType type)
{
    va_list list;
    int i;
	int numArgs = 0;
	ParseNode *node;
	
	va_start(list, type);
	while(va_arg(list, ParseNode*)) numArgs++;
	    
    va_start(list, type);
    
    node = (ParseNode*)malloc(sizeof(ParseNode));
    node->type = type;
    node->children = (ParseNode**)malloc(sizeof(ParseNode*) * numArgs);
    node->numChildren = numArgs;
    for(i = 0; i < numArgs; i++)
	    node->children[i] = va_arg(list, ParseNode*);	

	return node;
}

ParseNode *addChild(ParseNode *node, ParseNode *child)
{
	node->numChildren++;
	node->children = (ParseNode**)realloc(node->children, node->numChildren * sizeof(ParseNode*));
	
	node->children[node->numChildren - 1] = child;
	
	return node;
}
