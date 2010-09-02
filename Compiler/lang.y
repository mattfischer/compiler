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
%token EQUAL
%token END
%start s

%type <_node> statementlist statement expression add_expr mult_expr base_expr id
%%
s: statementlist END
	{ tree = $1; }

statementlist:	statementlist statement ';'
	{ $$ = addChild($1, $2); }
			  | statement ';'
	{ $$ = newNode(ParseNodeStatementList, $1, NULL);	}

statement: PRINT expression
	{ $$ = newNode(ParseNodePrint, $2, NULL); }
	     | id id
	{ $$ = newNode(ParseNodeVarDecl, $1, $2, NULL); }
		 | id '=' expression
    { $$ = newNode(ParseNodeAssign, $1, $3, NULL); }

expression: add_expr EQUAL add_expr
	{ $$ = newNode(ParseNodeEqual, $1, $3, NULL); }
		  | add_expr
	{ $$ = $1; }
	
add_expr: add_expr '+' mult_expr
	{ $$ = newNode(ParseNodeAdd, $1, $3, NULL); }
		  | mult_expr
	{ $$ = $1; }
	
mult_expr: mult_expr '*' base_expr
	{ $$ = newNode(ParseNodeMultiply, $1, $3, NULL); }
	| base_expr
	{ $$ = $1; }
	
base_expr: INT
	{ $$ = newNode(ParseNodeInt, NULL); $$->lexVal._int = $1; }
	  | id
	{ $$ = $1; }
	  | '(' expression ')'
	{ $$ = $2; }

id: ID
	{ $$ = newNode(ParseNodeId, NULL); $$->lexVal._id = $1; }

%%

extern FILE *langin;
extern int langline;

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
    node->line = langline;
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
