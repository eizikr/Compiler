#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct node{
	char *token;
	char* type;
	char* label;
	char* trueLabel;
	char* falseLabel;
	char* nextLabel;
	char* var;
	int line;
	int paramFlag;
	int oneStmt;
	char* code;
	struct node *left;
	struct node *right;
	struct node *parent;
}node;

node *mknode(char *token ,node *left, node *right);
char* strc(char* a, char* b);
void printtree(node *tree, int tab);
node *mk1node(char *token );
void printInOrderTree(node *tree, int tab);
void printPreOrderTree(node* tree,int tabs);

int isFUNC(char* a);
void printTabs(int tabs);
void printArgs(node* n,int tabs);
void printFunc(node* n, int tabs);
void printIf(node* n, int tabs);
void printIfElse(node* n, int tabs);
void printReturn(node* n, int tabs);
void printFuncCall(node* n, int tabs);
void printWhile(node* n, int tabs);
void printDoWhile(node* n, int tabs);
void printFor(node* n, int tabs);
void printAss(node* n, int tabs);
void printVarDec(node* n, int tabs);
void printStringDec(node* n,int tabs);
void printExpr(node*,int);
void addType(node* n,char* t);


void addType(node* n,char* t ){
	strcpy(n->type,t);
}

void printtree(node *tree,int tabs){

		if(strcmp(tree->token, "STMT") == 0 
		|| strcmp(tree->token, "IGNORE") == 0 
		|| strcmp(tree->token, "REC_FUNC") == 0 
		|| strcmp(tree->token, "DEC") == 0 )
			{
				if(tree->left)
					printtree(tree->left, tabs);
				if(tree->right)
					printtree(tree->right, tabs);
			}

		else{
			printTabs(tabs);
		
			if(strcmp(tree->token, "BLOCK") == 0)
			{
				printf("(%s\n" ,tree->token);
				if(tree->left)
					printtree(tree->left, tabs + 1);
				if(tree->right)
					printtree(tree->right, tabs + 1);
				printTabs(tabs);
				printf(")\n");
			}
			else if(isFUNC(tree->token))
				printFunc(tree,tabs+1);

			else if(strcmp(tree->token, "IF") == 0)
				printIf(tree,tabs+1);

			else if(strcmp(tree->token, "IF-ELSE") == 0)
				printIfElse(tree,tabs+1);

			else if(strcmp(tree->token, "FuncCall") == 0)
				printFuncCall(tree,tabs+1);
			
			else if(strcmp(tree->token, "WHILE") == 0)
				printWhile(tree,tabs+1);
			
			else if(strcmp(tree->token, "DO-WHILE") == 0)
				printDoWhile(tree,tabs+1);

			else if(strcmp(tree->token, "FOR") == 0)
				printFor(tree,tabs+1);
			
			else if(strcmp(tree->token, "=") == 0)
				printAss(tree,tabs);
			
			else if(strcmp(tree->token, "VARDEC") == 0)
				printVarDec(tree,tabs);
			
			else if(strcmp(tree->token, "STRING") == 0)
				printStringDec(tree,tabs);

			else{
				printf("\n");
			// printf("(%s\n" ,tree->token);

			// 	if(tree->left)
			// 		printtree(tree->left, tabs+1);
			// 	if(tree->right)
			// 		printtree(tree->right, tabs+1);
			// 	printTabs(tabs);
			// 	printf(")\n");
			printExpr(tree,tabs);
				
			}
		}
	
}

void printExpr(node* t, int tabs){
	node* n=t, *nl=t->left,*nr=t->right;

	if(strcmp(n->token,"FuncCall")==0){
		printtree(n,tabs);
	}
	else if(strcmp(n->token,"INDEX")==0)
	{
		printTabs(tabs);
		printf("[\n");
		printExpr(n->right,tabs);
		printTabs(tabs);
		printf("]\n");
	}
	else if(strcmp(n->token,"ABS")==0){
		printTabs(tabs);
		printf("ABS:\n");
		printtree(n->right,tabs+1);
	}
	else if(nl==NULL && nr==NULL){
		printTabs(tabs);
		printf("%s\n",n->token);
	}
	else if(nr && nl && nr->left==NULL && nr->right==NULL && nl->left==NULL && nl->right==NULL){
			printTabs(tabs);
			printf("(%s %s %s)\n",n->token,nl->token,nr->token);
	}
	else if(nl==NULL && nr){
		printTabs(tabs);
		printf("%s\n",n->token);
		if(nr->left==NULL && nr->right==NULL){
			printTabs(tabs+1);
			printf("%s\n",nr->token);
		}
		else
			printExpr(nr,tabs+1);
	}
	else{
		printTabs(tabs);
		printf("%s\n",n->token);
		if(nl)
			printExpr(nl,tabs+1);
		if(nr)
			printExpr(nr,tabs+1);
	}

}

void printInOrderTree(node* tree,int tabs)
{
	if(tree->left)
		printInOrderTree(tree->left, tabs);
	printf("%s" ,tree->token);
	if(tree->right)
		printInOrderTree(tree->right, tabs);
		
}

void printPreOrderTree(node* tree,int tabs)
{
	printf("%s " ,tree->token);
	if(tree->left)
		printPreOrderTree(tree->left, tabs);
	if(tree->right)
		printPreOrderTree(tree->right, tabs);
}



char* strc(char* a, char* b)
{
	char *newstr = (char*)malloc(sizeof(char)+1);
	newstr = strcpy(newstr,a);
	newstr = strcat(newstr,b);
	return newstr;
}

int isFUNC(char* a)
{
	char b[5] = "FUNC";
	for(int i=0;i<4;i++)
		if(a[i] != b[i])
			return 0;
	return 1;
}

void printTabs(int tabs)
{
	for(int i=0 ; i<tabs; i++)
		printf("\t");
}

void printArgs(node* n1,int tabs)
{
	node* n = n1;
	printf("(%s " ,n->token);
	n=n->left;
	if(n == NULL)
			printf("NONE)\n");
	else
	{	
		printf("\n");
		printTabs(tabs);
		printf("(%s ",n->token);
		n=n->left;
		while(n!=NULL)
		{	
			if(strcmp(n->token,"nextType") == 0)
				{	printf(")\n");
					printTabs(tabs);
					n=n->left;
					printf("(");
				} 
			if(n!=NULL)
			{
				printf("%s ",n->token);
				n=n->left;
			}
		}
		printf(")\n");
		printTabs(tabs-1);
		printf(")\n");
	}
}

void printFunc(node* n, int tabs)
{
	node* t = n;
	printf("(%s\n" ,t->token);
	printTabs(tabs);
	printf("FUNC NAME: %s\n",t->left->token);
	printTabs(tabs);
	printArgs(t->left->left,tabs+1); 				//sending the args node
	printTabs(tabs);
    printf("(BODY \n");
    if(t->left->right->left)
	    printtree(t->left->right->left,tabs+1);					//left->right gives us the body node
	printTabs(tabs);
    printf(")\n");
    if(t->right != NULL)
		printReturn(t->right,tabs);
	printTabs(tabs-1);
	printf(")\n");
}

void printReturn(node* n, int tabs)
{
	node*t = n;
	if(t->right != NULL)
	{
		printTabs(tabs);
		printf("(RETURN %s[\n", t->left->token);
	//	printInOrderTree(t->right->left,tabs);
		printExpr(t->right->left,tabs+2);		//print the index expr
		printTabs(tabs+1);
		printf("]\n");
		printTabs(tabs);
		printf(")\n");
	}
	else if(t->left->token[0] == '&')
	{
		printTabs(tabs);
		printf("(RETURN %s)\n", t->left->token);
	}
	else
	{
		printTabs(tabs);
		printf("(RETURN\n ");
		//printInOrderTree(t->left,tabs);
		printExpr(t->left,tabs+1);
		//printf("\n");
		printTabs(tabs);
		printf(")\n");
	}
		
}

void printIf(node* n, int tabs)
{
	node* t = n;
	printf("(%s\n" ,t->token);
	printTabs(tabs);
	printf("(COND:\n ");
	//printInOrderTree(t->left,tabs);
	printExpr(t->left,tabs+1);
	printTabs(tabs);
	printf(")\n");
	printtree(t->right,tabs);
	printTabs(tabs-1);
	printf(")\n");
}


void printIfElse(node* n, int tabs)
{
	node* t = n;
	printf("(%s\n" ,t->token);
	printTabs(tabs);
	printIf(t->left,tabs+1);
	printTabs(tabs);
	printf("(%s\n" ,t->right->token);
	printtree(t->right->left,tabs+1); 		//check later  if else not null
	printTabs(tabs);
	printf(")\n");
	printTabs(tabs-1);
	printf(")\n");
}

void printFuncCall(node* n, int tabs)
{
	node* t = n;
	printf("(FUNC_CALL: %s\n",t->left->token);
	t=t->right;
	printTabs(tabs);
	if(strcmp(t->token,"PARAMS_NONE") == 0)
		printf("(PARAMS: NONE)\n");
	else
		{
			t=t->left;			// moving from pramas val to funcexp
			printf("(PARAMS:\n ");
			
			while(t!=NULL){
					if(strcmp(t->token,",")==0)		//checking if the node is ',' to know how to act
					{
						printExpr(t->left,tabs+1);	
						t=t->right;
					}
					else{
						printExpr(t,tabs+1);
						t=NULL;
					}
			}
			printTabs(tabs);
			printf(")\n");
		}
	printTabs(tabs-1);
	printf(")\n");
}

void printWhile(node* n, int tabs)
{
	node* t = n;
	printf("(WHILE: \n");
	printTabs(tabs);
	printf("(COND: \n");
	printExpr(t->left,tabs+1);
	//printtree(t->left,tabs+1);
	printTabs(tabs);
	printf(")\n");
	printtree(t->right,tabs);
	printTabs(tabs-1);
	printf(")\n");
}

void printDoWhile(node* n, int tabs)
{
	node* t = n;
	printf("(DO_WHILE: \n");
	printtree(t->left,tabs);
	printTabs(tabs);
	printf("(COND: \n");
	printExpr(t->right,tabs+1);
	//printtree(t->right,tabs+1);
	printTabs(tabs);
	printf(")\n");
	printTabs(tabs-1);
	printf(")\n");
}

void printFor(node* n, int tabs)
{
    node *init,*update,*cond,*t;
    t=n;
	printf("(FOR: \n");
	printTabs(tabs);
    init = t->left;
    cond = init->right;
    update = cond->right;
    printf("(INIT: %s = %s)\n",init->left->left->token,init->left->right->token);
	printTabs(tabs);
    printf("(COND: \n");
	printExpr(cond->left,tabs+1);
	//printtree(cond->left,tabs+1);
	printTabs(tabs);
	printf(")\n");
	printTabs(tabs);
    printf("(UPDATE: \n");
	printTabs(tabs+1);
    printf("%s = \n",update->left->token); 
	printExpr(update->right,tabs+1);                
//	printtree(update->right,tabs+1);                
	printTabs(tabs);
	printf(")\n");
    printtree(t->right,tabs);
    printTabs(tabs-1);
	printf(")\n");
}

void printAss(node* n, int tabs)
{
    node* t = n;
    printf("(");
    if(t->left->left == NULL)
        printf("%s=\n",t->left->token);
    else
    {
        printf("%s[\n",t->left->token);
        printtree(t->left->left->left,tabs+1);
        printTabs(tabs);
        printf("]=\n");
    }
    if(t->right->left && strcmp(t->right->left->token,"INDEX") == 0)
    {
        printTabs(tabs+1);
        printf("%s[\n",t->right->token);
        printtree(t->right->left->left,tabs+2);
        printTabs(tabs+1);
        printf("]\n"); 
    }
    else
        printtree(t->right,tabs+1);
        
    printTabs(tabs);
    printf(")\n");
}

void printVarDec(node* n, int tabs)
{
	node *nv,*t = n;
	printf("(VARDEC: %s\n",t->left->token);				//print the type
	printTabs(tabs+1);
	printf("(%s ",t->left->left->token);		//print the first id
	nv = t->left->right;						//next var ptr
	while(nv != NULL)	{				//if nextvar is not null
		if(strcmp(nv->token,"=")==0){
			printf("=\n");
			printExpr(nv->left,tabs+2);
			nv=nv->right;
		}
		else //token=id
		{	
			printf("\n");
			printTabs(tabs+1);
			printf("%s", nv->token);
			nv=nv->left;
		}

	}
	printf("\n");
	printTabs(tabs+1);
	printf(")\n");	
	printTabs(tabs);
	printf(")\n");	
}

void printStringDec(node* n,int tabs){
	node *ns,*t = n;
	printf("(STRING_DEC:\n");				//print the id
	printTabs(tabs+1);
	printf("(%s[%s]",t->left->token,t->left->left->token);		//print the size
	ns = t->right;						//next var ptr
	while(ns != NULL)	//if next str is not null
	{
		if(strcmp(ns->token,"=")==0){
			printf("=%s",ns->left->token);

		}
		else //token=id
		{
			printf(", %s[%s]", ns->token,ns->left->token);
		}
		ns=ns->right;

	}
	printf(")\n");	
	printTabs(tabs);
	printf(")\n");	
}






