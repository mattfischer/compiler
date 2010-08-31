%option noyywrap

%{
	#include <malloc.h>
	#include <stdlib.h>
	
	#include "ParseNode.h"
	
	#include "lang.tab.h"
%}

INT [[:digit:]]+
ID [[:alpha:]]+

%%
[[:blank:]]*
\n(\n|[[:blank:]])* return NEWL;
print return PRINT;
{INT} langlval._int = atoi(langtext); return INT;
{ID} langlval._id = malloc(strlen(langtext) + 1); strcpy(langlval._id, langtext); return ID;
(\+|\*|\(|\)) return *langtext;

<<EOF>> return END;
%%