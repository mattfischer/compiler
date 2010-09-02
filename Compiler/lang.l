%option noyywrap

%{
	#include <malloc.h>
	#include <stdlib.h>
	
	#include "ParseNode.h"
	
	#include "lang.tab.h"
	
	int langline = 1;
%}

INT [[:digit:]]+
ID [[:alpha:]]+

%%
[[:blank:]]*
\n langline++;

print return PRINT;
{INT} langlval._int = atoi(langtext); return INT;
{ID} langlval._id = malloc(strlen(langtext) + 1); strcpy(langlval._id, langtext); return ID;
== return EQUAL;
(\+|\*|\(|\)|=|;) return *langtext;

<<EOF>> return END;
%%