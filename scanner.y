%{
#include "astFuncs.c"
#include "lex.yy.c"
char* e_msg = "s", *NO_RETURN = "missing return";
int flag;
%}

%union
{
	struct node* node;
	char* str;
	}

%token <str> INTVAL STRINGVAL REALVAL BOOL VAR ID NULLL CHAR INT REAL STRING INTPTR CHARPTR REALPTR VOID
%token <str> IF ELSE WHILE FOR RETURN DO
%token <str> AND DIV EQL EQLEQL BIGGER BIGGEREQL SMALLER SMALLEREQL MINUS NOT NOTEQL OR PLUS MUL ADDR CHARVAL
%token <str> TRUE FALSE 

%left OR
%left AND
%left EQL EQLEQL BIGGER BIGGEREQL SMALLER SMALLEREQL NOTEQL NOT
%left PLUS MINUS
%left DIV MUL

%nonassoc then 
%nonassoc ELSE 

%type <node> EXPR FUNCCALL CODE FUNCEXPR ASS DECLERATION BLOCK VARDEC NEXTVAR DECASS STRARR
%type <node> NEXTSTRARR ASSIGNSTR STATEMENT STATMENTS STATMENTS_FUNC IF_ST WHILE_ST DO_WHILE_ST FOR_ST RETURN_ST 
%type <node> FUNC SUB_FUNC ARGS NEXTARG BODY BODY_FUNC FUNCS opt_STR
%type <str> VALUE TYPE 

%%
CODE:FUNCS {printf("(CODE:\n");
			printtree($$, 1);
			printf(")\n");
			}

FUNCS: FUNC FUNCS				{$$ = mknode("REC_FUNC",$1,$2);}
		| FUNC					{$$ = $1;}
		;
	
FUNC: 
	VOID ID '(' ARGS ')' '{' BODY '}'			
	{$$ = mknode("FUNC: VOID",mknode($2,$4,mknode("BODY",$7,NULL)),NULL);}
	|SUB_FUNC									{$$ = $1;}
	;		

SUB_FUNC:TYPE ID '(' ARGS ')' '{' BODY_FUNC  '}'	{$$ = mknode(strc("FUNC: ",$1),mknode($2,$4,mknode("BODY",$7,NULL)),NULL);}
	;

BODY:
	VARDEC BODY									{$$ = mknode("IGNORE",$1,$2);}
	|FUNC BODY									{$$ = mknode("IGNORE",$1,$2);}
	|STRARR BODY								{$$ = mknode("IGNORE",$1,$2);}
	|STATMENTS									{$$ = $1;}
	;

BODY_FUNC:
	VARDEC BODY_FUNC							{$$ = mknode("IGNORE",$1,$2);}
	|FUNC BODY_FUNC								{$$ = mknode("IGNORE",$1,$2);}
	|STRARR BODY_FUNC							{$$ = mknode("IGNORE",$1,$2);}
	|STATMENTS_FUNC								{$$ = $1;}

ARGS: 
	TYPE ID NEXTARG								{$$ = mknode("ARGS",mknode($1,mknode($2,$3,NULL),NULL),NULL);}
	|											{$$ = mk1node("ARGS");}
	;

TYPE: INT										{$$ = "INT";}	
	|INTPTR										{$$ = "INT*";}	
	|REAL										{$$ = "REAL";}	
	|REALPTR									{$$ = "REAL*";}	
	|CHAR										{$$ = "CHAR";}	
	|CHARPTR									{$$ = "CHAR*";}	
	|BOOL										{$$ = "BOOL";}	
	;

NEXTARG: 
		';' TYPE ID NEXTARG						{$$ = mknode("nextType",mknode($2,mknode($3,$4,NULL),NULL),NULL);}
		|',' ID NEXTARG							{$$ = mknode($2,$3,NULL);}
		|										{$$ = NULL;}
		;

VARDEC: VAR TYPE ID NEXTVAR ';'					{$$ = mknode("VARDEC",mknode($2,mk1node($3),$4),NULL);}
		;

NEXTVAR: ',' ID NEXTVAR							{$$ = mknode($2,$3,NULL);}
		|DECASS									{$$ = $1;}
		|										{$$ = NULL;}
		;				

DECASS: EQL EXPR NEXTVAR						{$$ = mknode("=",$2,$3);}
		;

VALUE: TRUE|FALSE|REALVAL|INTVAL|CHARVAL|ID|NULLL;

STRARR: STRING ID '[' INTVAL ']' NEXTSTRARR ';' {$$ = mknode("STRING",mknode($2,mk1node($4),NULL),$6);}
	;			

NEXTSTRARR: ',' ID '[' INTVAL ']' NEXTSTRARR	{$$ = mknode($2,mk1node($4),$6);}
			|ASSIGNSTR							{$$ = $1;}
			|									{$$ = NULL;}
			;

ASSIGNSTR: EQL STRINGVAL NEXTSTRARR				{$$ = mknode("=",mk1node($2),$3);}
			;		

STATMENTS:								
			STATEMENT STATMENTS					{$$ = mknode("STMT",$1,$2);}			
			|									{$$=NULL;}
			;

STATMENTS_FUNC: STATEMENT STATMENTS_FUNC		{$$ = mknode("STMT",$1,$2);}
				|RETURN_ST ';'					{$$ = $1;}
			;

STATEMENT:	
			ASS ';'								{$$ = $1;}
			|BLOCK								{$$ = $1;}
			|FUNCCALL ';'						{$$ = $1;}
			|IF_ST								{$$ = $1;}
			|WHILE_ST							{$$ = $1;}
			|DO_WHILE_ST ';'					{$$ = $1;}
			|FOR_ST								{$$ = $1;}
			|RETURN_ST ';'						{$$ = $1;}
			;

ASS: ID EQL EXPR							{$$ = mknode("=",mk1node($1),$3);}
	|ID '[' EXPR ']' EQL CHARVAL			{$$ = mknode("=",mknode($1,mknode("INDEX",$3,NULL),NULL),mk1node($6));}
	|ID '[' EXPR ']' EQL ID '[' EXPR ']'	{$$ = mknode("=",mknode($1,mknode("INDEX",$3,NULL),NULL),mknode($6,mknode("INDEX",$8,NULL),NULL));}
	|ID '[' EXPR ']' EQL FUNCCALL			{$$ = mknode("=",mknode($1,mknode("INDEX",$3,NULL),NULL),$6);}
	|ID '[' EXPR ']' EQL ID					{$$ = mknode("=",mknode($1,mknode("INDEX",$3,NULL),NULL),mk1node($6));}
	|MUL ID EQL EXPR						{$$ = mknode("=",mk1node(strc("*",$2)),$4);}
	|ID EQL STRINGVAL						{$$ = mknode("=",mk1node($1),mk1node($3));}
	;
	
BLOCK:'{' DECLERATION '}'					{$$ = mknode("BLOCK",$2,NULL);}
	;

DECLERATION: VARDEC DECLERATION				{$$ = mknode("DEC",$1,$2);}
			|STRARR DECLERATION				{$$ = mknode("DEC",$1,$2);}
			|SUB_FUNC						{$$ = $1;}					
			|STATMENTS						{$$ = $1;}
			;

FUNCCALL:ID '(' FUNCEXPR ')' 						{$$ = mknode("FuncCall",mk1node($1),mknode("PARAMS_VAL",$3,NULL));}		
		|ID '(' ')'									{$$ = mknode("FuncCall",mk1node($1),mk1node("PARAMS_NONE"));}
		;

FUNCEXPR:EXPR										{$$ = $1;}
		|EXPR ',' FUNCEXPR							{$$ = mknode(",",$1,$3);}
		;			

IF_ST: IF '(' EXPR ')' STATEMENT %prec then			{$$ = mknode("IF",$3,$5);}
		|IF '(' EXPR ')' STATEMENT ELSE STATEMENT	{$$ = mknode("IF-ELSE",mknode("IF",$3,$5),mknode("ELSE",$7,NULL));}
		;

WHILE_ST: WHILE '(' EXPR ')' STATEMENT				{$$ = mknode("WHILE",$3,$5);}
		;

DO_WHILE_ST: DO BLOCK WHILE '(' EXPR ')'			{$$ = mknode("DO-WHILE",$2,$5);}
			;

FOR_ST: FOR '(' ID EQL INTVAL ';' EXPR ';' ID EQL EXPR ')' STATEMENT		
	{$$ = mknode("FOR",mknode("INIT",mknode("=",mk1node($3),mk1node($5)),mknode("COND",$7,mknode("=",mk1node($9),$11))),$13);}
		;

RETURN_ST:
			RETURN EXPR								{$$ = mknode("RETURN",$2,NULL);}
			;

EXPR: 
	 EXPR PLUS EXPR				{$$ = mknode("+",$1,$3);}
	|EXPR MINUS EXPR			{$$ = mknode("-",$1,$3);}
	|EXPR DIV EXPR  			{$$ = mknode("/",$1,$3);}
	|EXPR MUL EXPR				{$$ = mknode("*",$1,$3);}
	|EXPR BIGGER EXPR			{$$ = mknode(">",$1,$3);}
	|EXPR SMALLER EXPR			{$$ = mknode("<",$1,$3);}
	|EXPR BIGGEREQL EXPR		{$$ = mknode(">=",$1,$3);}
	|EXPR SMALLEREQL EXPR		{$$ = mknode("<=",$1,$3);}
	|EXPR EQLEQL EXPR			{$$ = mknode("==",$1,$3);}
	|EXPR NOTEQL EXPR			{$$ = mknode("!=",$1,$3);}
	|EXPR AND EXPR				{$$ = mknode("&&",$1,$3);}
	|EXPR OR EXPR				{$$ = mknode("||",$1,$3);}
	|NOT EXPR					{$$ = mknode("!",NULL,$2);}
	|MUL EXPR					{$$ = mknode("*",NULL,$2);}
	|PLUS EXPR					{$$ = mknode("+",NULL,$2);}
	|MINUS EXPR					{$$ = mknode("-",NULL,$2);}
	|ADDR ID opt_STR			{$$ = mknode("&",NULL,mknode($2,NULL,$3));}
	|ID '[' EXPR ']'			{$$ = mknode($1,NULL,mknode("INDEX",NULL,$3));}
	|'(' EXPR ')'				{$$ = $2;}
	|'|' ID '|'					{$$ = mknode("STRLEN:",NULL,mk1node($2));}
	|VALUE						{$$ = mknode($1,NULL,NULL);}
	|FUNCCALL					{$$ = $1;}
	;

opt_STR:'[' EXPR ']'			{$$ = mknode("INDEX",NULL,$2);}
	| 							{$$=NULL;}
%%

void main(){
	yyparse();
}

int yyerror(char *s){
	fprintf(stderr,"line %d: \033[0;31m%s\033[0m, unexpected: %s\n", yylineno, s, yytext);
	return 0;
}

int yywarp(){
	return 1;
}