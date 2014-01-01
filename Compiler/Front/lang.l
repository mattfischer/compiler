%option noyywrap

%{
	#include <malloc.h>
	#include <stdlib.h>
	#include <string.h>

	#define YY_NO_UNISTD_H

	#include "Front/ParseNode.h"
	
	#include "lang.tab.h"
	
	int langline = 1;
%}

INT [[:digit:]]+
ID [[:alpha:]]+

%%
[[:blank:]]*
\n langline++;

print return PRINT;
if return IF;
else return ELSE;
while return WHILE;
return return RETURN;
== return EQUAL;
!= return NEQUAL;
(\+|\*|\(|\)|=|;|\{|\}) return *langtext;
{INT} langlval._int = atoi(langtext); return INT;
{ID} langlval._id = malloc(strlen(langtext) + 1); strcpy(langlval._id, langtext); return ID;

<<EOF>> return END;
%%