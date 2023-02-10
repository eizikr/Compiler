%{
#include <stdio.h>
#include "y.tab.h"
%}

%%

"bool"	return BOOL;
"char"	return CHAR;
"int"	return INT;
"real"	return REAL;
"string"	return STRING;
"int*"	return INTPTR;
"char*"	return CHARPTR;
"real*"	return REALPTR;
"var"	return VAR;
"void"	return VOID;
"null"	{yylval.str = "null"; return NULLL;}
"return" return RETURN;

true	{yylval.str = "true"; return TRUE;}
false	{yylval.str = "false"; return FALSE;}

"if"	return IF;
"else"	return ELSE;
"while"	return WHILE;
"for"	return FOR;
"do"	return DO;

"&&"	return AND;
"/"	return DIV;
"=="	return EQLEQL;
"="	return EQL;
">="	return BIGGEREQL;
">"	return BIGGER;
"<="	return SMALLEREQL;
"<"	return SMALLER;
"-"	{return MINUS;}
"!="	return NOTEQL;
"!"	return NOT;
"||"	return OR;
"+"	{return PLUS;}
"*"	{return MUL;}
"&"	return ADDR;

[a-zA-Z][a-zA-Z0-9_]*	{yylval.str = strdup(yytext); return ID;}

[0-9]+			    {yylval.str = strdup(yytext); return INTVAL;}
[0-9]+[.][0-9]+	{yylval.str = strdup(yytext); return REALVAL;}
[0][xX][0-9]+[A-F]	{yylval.str = strdup(yytext); return INTVAL;}


['][!-~][']	{yylval.str = strdup(yytext); return CHARVAL;}

["][ !#-~]*["]	{yylval.str = strdup(yytext); return STRINGVAL;}

[0-9]*[.]?[0-9]*[Ee][+-]?[0-9]*	{yylval.str = strdup(yytext); return REALVAL;}


"/*"([\n -)+-.0-~][/*]?[\n -)+-.0-~]+[/*]?[\n -)+-.0-~])*"*/"	/* Ignore Commands */; 

" "|"\t" ;

"\n"        {yylineno++;}

. return yytext[0];

%%

