%option noyywrap

%{
	#include <malloc.h>
	#include <stdlib.h>
	
	#include "ParseNode.h"
	
	#include "lang.tab.h"
%}

INT [[:digit:]]+

%%
[[:blank:]]*
\n(\n|[[:blank:]])* return NEWL;
print return PRINT;
{INT} langlval._int = atoi(langtext); return INT;
(\+|\*|\(|\)) return *langtext;

<<EOF>> return END;
%%