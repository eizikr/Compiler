%{
#include "lex.yy.c"

#include "TAC.c"
extern int yylineno;
char* e_msg = "s", *NO_RETURN = "missing return";
int flag;

node* mknode(char *token ,node *left ,node *right){
	node *newnode = (node*)malloc(sizeof(node));
	char *newstr = (char*)malloc(sizeof(char)+1);
	newnode->type=(char*)malloc(sizeof(char)+1);

	newnode->code=(char*)malloc(sizeof(char)+200);
	strcpy(newnode->code,"");
	//labels:
	newnode->label=(char*)malloc(sizeof(char)+1);
	newnode->trueLabel=(char*)malloc(sizeof(char)+1);
	newnode->falseLabel=(char*)malloc(sizeof(char)+1);
	newnode->nextLabel=(char*)malloc(sizeof(char)+1);
	newnode->var=(char*)malloc(sizeof(char)+1);
	strcpy(newnode->var,"no_var");

	strcpy(newnode->type,"");
	strcpy(newstr ,token);
	newnode->left = left;
	newnode->right = right;
	newnode->token = newstr;
	newnode->line = yylineno;
	newnode->paramFlag=0;
	newnode->oneStmt=0;
	return newnode;
}
node *mk1node(char *token )
{
	return mknode(token,NULL,NULL);
}

node* addParent(node* child, node* parent)
{
	child->parent = parent;
	return child;
}

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
%type <node> FUNC SUB_FUNC ARGS NEXTARG BODY BODY_FUNC FUNCS opt_STR VALUE
%type <str>  TYPE 



%%
CODE:	FUNCS{
			codeTables=buildCode(codeTables);
			semanticsScan(codeTables,$$);
			checkMainExists(codeTables);
			
			printf("(CODE:\n");
			printtree($$, 1);
			printf(")\n");
			
			//3ac (part3):
			addLabelsToTree($$);
			secondScan($$);
			//printT($$);
			printf("%s\n",$$->code);
			writeToFIle($$->code);
			}


FUNCS: FUNC FUNCS				{$$ = mknode("REC_FUNC",$1,$2);}
		| FUNC					{$$ = $1;}
		;

	
FUNC: 
	VOID ID '(' ARGS ')' '{' BODY '}'			{node* funcID= mknode($2,$4,mknode("BODY",$7,NULL));
												addType(funcID,"FUNCID");
												$$ = mknode("FUNC: VOID",funcID,NULL);}
	|SUB_FUNC									{$$ = $1;}
	;		

SUB_FUNC:TYPE ID '(' ARGS ')' '{' BODY_FUNC  '}'	{node* funcID= mknode($2,$4,mknode("BODY",$7,NULL));
													addType(funcID,"FUNCID");
													$$ = mknode(strc("FUNC: ",$1),funcID,NULL);}
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
	TYPE ID NEXTARG								{node* id = mknode($2,$3,NULL);
												addType(id,"ID");
												$$ = mknode("ARGS",mknode($1,id,NULL),NULL);}
	|											{$$ = mk1node("ARGS");}
	;

TYPE: INT										{$$ = "int"; }	
	|INTPTR										{$$ = "int*";}	
	|REAL										{$$ = "real";}	
	|REALPTR									{$$ = "real*";}	
	|CHAR										{$$ = "char";}	
	|CHARPTR									{$$ = "char*";}	
	|BOOL										{$$ = "bool";}	
	;

NEXTARG: 
		';' TYPE ID NEXTARG						{node* id = mknode($3,$4,NULL);
												addType(id,"ID");
												$$ = mknode("nextType",mknode($2,id,NULL),NULL);}
		|',' ID NEXTARG							{$$ = mknode($2,$3,NULL); addType($$,"ID");}
		|										{$$ = NULL;}
		;

VARDEC: VAR TYPE ID NEXTVAR ';'					{node* id= mk1node($3);
												addType(id,"ID");
												node* x=mknode($2,id,$4);
												$$ = mknode("VARDEC",x,NULL);
												if($4)
													$4=addParent($4,x);
												id=addParent(id,x);
												}
		;

NEXTVAR: ',' ID NEXTVAR							{$$ = mknode($2,$3,NULL);addType($$,"ID");
												if($3)
													$3=addParent($3,$$);
												}
		|DECASS									{$$ = $1;
												}
		|										{$$ = NULL;}
		;				

DECASS: EQL EXPR NEXTVAR						{$$ = mknode("=",$2,$3);
												if($3)
													$3=addParent($3,$$);
												}
		;

VALUE: TRUE			{$$ = mk1node($1); addType($$,"bool");}
	   |FALSE		{$$ = mk1node($1); addType($$,"bool");}
	   |REALVAL		{$$ = mk1node($1); addType($$,"real");}
	   |INTVAL		{$$ = mk1node($1); addType($$,"int");}
	   |CHARVAL		{$$ = mk1node($1); addType($$,"char");}
	   |ID			{$$ = mk1node($1); addType($$,"ID");}
	   |NULLL		{$$ = mk1node($1); addType($$,"NULL");}
	   ;

STRARR: STRING ID '[' INTVAL ']' NEXTSTRARR ';' {node* intval=mk1node($4); addType(intval,"int");
												node* id= mknode($2,intval,NULL);
												$$ = mknode("STRING",id,$6);
												if($6)
													$6=addParent($6,$$);
												addType(id,"ID");
												}
	;			

NEXTSTRARR: ',' ID '[' INTVAL ']' NEXTSTRARR	{node* intval=mk1node($4); addType(intval,"int");
												$$ = mknode($2,intval,$6); addType($$,"ID");
												if($6)
													$6=addParent($6,$$);}
			|ASSIGNSTR							{$$ = $1;}
			|									{$$ = NULL;}
			;

ASSIGNSTR: EQL STRINGVAL NEXTSTRARR				{node* strval= mk1node($2);
												addType(strval,"string");
												$$ = mknode("=",strval,$3);
												if($3)
													$3=addParent($3,$$);
												}
			;		

STATMENTS:								
			STATEMENT STATMENTS					{$$ = mknode("STMT",$1,$2); $1 = addParent($1,$$);}			
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
			


ASS: ID EQL EXPR							{
												node* id = mk1node($1);
												addType(id,"ID");
												$$ = mknode("=",id,$3);
											}
	|ID '[' EXPR ']' EQL EXPR				{	
												node* n = mknode($1,mknode("INDEX",$3,NULL),NULL);
												addType(n,"ID");
												$$ = mknode("=",n,$6);
											}
	|MUL ID EQL EXPR						{
												//node* n = mk1node(strc("*",$2));
												//addType(n,"*ID");
												node* n =mk1node($2); addType(n,"ID");
												$$ = mknode("=",mknode("*",NULL,n),$4);
											}
	|ID EQL STRINGVAL						{	node* n=mk1node($1); addType(n,"ID");
												node* strval = mk1node($3);
												addType(strval,"string");
												$$ = mknode("=",n,strval);
											}
	;
	
BLOCK:'{' DECLERATION '}'					{$$ = mknode("BLOCK",$2,NULL);
											if($2)	
												$2 = addParent($2,$$);
											}
	;

DECLERATION: VARDEC DECLERATION				{$$ = mknode("DEC",$1,$2);}
			|STRARR DECLERATION				{$$ = mknode("DEC",$1,$2);}
			|SUB_FUNC						{$$ = $1;}					
			|STATMENTS						{$$ = $1;}
			;
FUNCCALL:ID '(' FUNCEXPR ')' 						{node* id=mk1node($1); addType(id,"ID");
													$$ = mknode("FuncCall",id,mknode("PARAMS_VAL",$3,NULL));}		
		|ID '(' ')'									{node* id=mk1node($1); addType(id,"ID");
													$$ = mknode("FuncCall",id,mk1node("PARAMS_NONE"));}
		;

FUNCEXPR:EXPR										{$$ = $1; $$->paramFlag=1;}
		|EXPR ',' FUNCEXPR							{$$ = mknode(",",$1,$3);$1->paramFlag=1;}
		;			

IF_ST: IF '(' EXPR ')' STATEMENT %prec then			{$$ = mknode("IF",$3,$5);
													if($5 && strcmp($5->token,"BLOCK")!=0 && strcmp($5->token,"=")!=0){
														$5->oneStmt=1;
													}
														}
		|IF '(' EXPR ')' STATEMENT ELSE STATEMENT	{	node* n = mknode("IF",$3,$5);
														$$ = mknode("IF-ELSE",n,mknode("ELSE",$7,NULL));
														n = addParent(n,$$);
														if($5 && strcmp($5->token,"BLOCK")!=0 && strcmp($5->token,"=")!=0){
															$5->oneStmt=1;
														}
														if($7 && strcmp($7->token,"BLOCK")!=0 && strcmp($7->token,"=")!=0){
															$7->oneStmt=1;
														}
														}
		;

WHILE_ST: WHILE '(' EXPR ')' STATEMENT				{$$ = mknode("WHILE",$3,$5);
													if($5 && strcmp($5->token,"BLOCK")!=0 && strcmp($5->token,"=")!=0){
														$5->oneStmt=1;
													}
													}
		;

DO_WHILE_ST: DO BLOCK WHILE '(' EXPR ')'			{$$ = mknode("DO-WHILE",$2,$5);}
			;

FOR_ST: FOR '(' ID EQL INTVAL ';' EXPR ';' ID EQL EXPR ')' STATEMENT		
		{node* intval=mk1node($5); addType(intval,"int");
		node* id1=mk1node($3); addType(id1,"ID");
		node* id2=mk1node($9); addType(id2,"ID");
		$$ = mknode("FOR",mknode("INIT",mknode("=",id1,intval),mknode("COND",$7,mknode("=",id2,$11))),$13);
		if($13 && strcmp($13->token,"BLOCK")!=0 && strcmp($13->token,"=")!=0){
			$13->oneStmt=1;}
		}
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
	|ADDR ID opt_STR			{node* id=mknode($2,NULL,$3);
								addType(id,"ID");
								$$ = mknode("&",NULL,id);
								id=addParent(id,$$);

								}
	|ID '[' EXPR ']'			{$$ = mknode($1,NULL,mknode("INDEX",NULL,$3));
								addType($$,"ID");}
	|'(' EXPR ')'				{$$ = $2;}
	|'|' ID '|'					{node* id=mk1node($2);
								addType(id,"ID");
								$$ = mknode("STRLEN:",NULL,id);}
	|FUNCCALL					{$$ = $1;}
	|VALUE						{$$ = $1;}
	;

opt_STR:'[' EXPR ']'			{$$ = mknode("INDEX",NULL,$2);}
	| 							{$$=NULL;}
%%


void main(){
	yyparse();
}

int yyerror(char *s){
	fprintf(stderr,"\033[0;31m%s line %d:\033[0m unexpected: %s\n", s, yylineno, yytext);
	return 0;
}

int yywarp(){
	return 1;
}
