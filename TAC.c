
#include <stdlib.h>
#include "Semantics.c"

int label_num = 0;
int temp_index = 0;
void addLabelsToTree(node *tree);

char *freshVar();
char *freshLabel();
void resetLabel();
void secondScan(node *tree);
void calcInherited(node *tree);
int oneStmt(node* tree);
void calcCode(node *tree);
void calcIf(node *tree);
void calcWhile(node *tree);
void calcDoWhile(node *tree);
void calcFor(node *tree);
void calcFuncCall(node *tree);
void calcFunc(node* tree);
void calcVarDec(node* tree);
void pushParams(node* param,node* tree);
void expr_scan2(node *tree);
void gen(node *tree, char *var, char *op, char *left, char *right);
void cat2sons(node *tree);
char *reSize(char *ptr, int size);
			
void writeToFIle(char* TACcode);

void expr_scan2(node *tree)
{
    node *b1 = tree->left, *b2 = tree->right;
    if (strcmp(tree->token, "FuncCall") == 0)
    {
        calcFuncCall(tree);
    }
    else if (strcmp(tree->token, "INDEX") == 0)
    {
        node *index;
        if (tree->right)
            index = tree->right;
        else
            index = tree->left;
        strcpy(tree->var, index->var);

        tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(index->code) + 1);
        strcat(tree->code, index->code);
    }
    else if (strcmp(tree->type, "ID") == 0)
    { 
        //tree->right -> index
        node *index;
        if (tree->right)
        {
            index = tree->right;
        }
        else
            index = tree->left;
        if (index && strcmp(index->token,"INDEX")==0)
        {
            char *temp = (char *)malloc(sizeof(char) + 1);
            char *temp2 = (char *)malloc(sizeof(char) + 1);

            strcpy(tree->var, freshVar());
            tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(index->code) + 1);
            strcat(tree->code, index->code);              // index code is t1=exp in []
            gen(tree, tree->var, "&", NULL, tree->token); //t2= &id
            strcpy(temp, freshVar());                     // new var for t3 = t2+t1
            gen(tree, temp, "+", tree->var, index->var);  // add code: t3 = t2+t1
            strcpy(tree->var, temp);                      // tree var is t3
            if (tree->right  && !(tree->parent && strcmp(tree->parent->token,"&")==0) )
            {
                strcpy(temp2, tree->var);
                strcpy(tree->var, freshVar());
                gen(tree, tree->var, "*", NULL, temp2);
            }
        }
        else
        {   
            
            strcpy(tree->var, tree->token);
            if(tree->parent && strcmp(tree->parent->token,"&")!=0){
                cat2sons(tree);
            }

        }
    }
    else if (tree->left == NULL && tree->right)
    { //node=operator,null,exp

        if (strcmp(tree->token, "!") == 0) //operator:! ,the expr must be bool
        {
            strcpy(tree->var, freshVar());
            cat2sons(tree);
            gen(tree, tree->var, tree->token, NULL, tree->right->var);
        }

        else if (strcmp(tree->token, "STRLEN:") == 0)
        {
        }

        else if (strcmp(tree->token, "-") == 0 || strcmp(tree->token, "+") == 0)
        {
            strcpy(tree->var, freshVar());
            cat2sons(tree);
            gen(tree, tree->var, tree->token, NULL, tree->right->var);
        }

        else if (strcmp(tree->token, "&") == 0 )
        {   //&id ----> freshvar= &id , tree->var=fresh var 
            cat2sons(tree);
            node *index=NULL;
            if (tree->right->right)
            {
                index = tree->right->right;
            }
            else
                index = tree->right->left;
            if (index==NULL){
                strcpy(tree->var,freshVar());
                gen(tree,tree->var,"&",NULL,tree->right->var); // tree->right is id
            }
            else{
                strcpy(tree->var,tree->right->var);
            }
        }
        else if(strcmp(tree->token,"*")==0){

            strcpy(tree->var,freshVar());
            cat2sons(tree);
            gen(tree,tree->var,"*",NULL,tree->right->var); // tree->right is id
        }
    }
    else if (tree->left && tree->right)
    { //node=exp,operator,exp
        //operator:+,-,*,/
        if (strcmp(tree->token, "+") == 0 || strcmp(tree->token, "-") == 0 || strcmp(tree->token, "*") == 0 || strcmp(tree->token, "/") == 0)
        {
            strcpy(tree->var, freshVar());
            cat2sons(tree);
            gen(tree, tree->var, tree->token, tree->left->var, tree->right->var);
        }
        //operator: >,<,>=,<=
        else if (strcmp(tree->token, ">") == 0 || strcmp(tree->token, "<") == 0 || strcmp(tree->token, ">=") == 0 || strcmp(tree->token, "<=") == 0 || strcmp(tree->token, "==") == 0 || strcmp(tree->token, "!=") == 0)
        {
            cat2sons(tree);
            tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(tree->left->var) + strlen(tree->token) + strlen(tree->right->var) + strlen(tree->trueLabel) + strlen(tree->falseLabel) + 25);
            strcat(tree->code, "\tif ");
            strcat(tree->code, tree->left->var);
            strcat(tree->code, tree->token);
            strcat(tree->code, tree->right->var);
            strcat(tree->code, " goto ");
            strcat(tree->code, tree->trueLabel);
            strcat(tree->code, "\n\tgoto ");
            strcat(tree->code, tree->falseLabel);
            strcat(tree->code, "\n");
        }
        // Operator: ||, &&
        else if (strcmp(tree->token, "&&") == 0 || strcmp(tree->token, "||") == 0) 
        {
            tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(b1->code) + strlen(b1->trueLabel) + strlen(b2->code) + 10);
            strcat(tree->code, b1->code);
            if (strcmp(tree->token, "&&") == 0)
                strcat(tree->code, b1->trueLabel);
            else
                strcat(tree->code, b1->falseLabel);
            strcat(tree->code, ":");
            strcat(tree->code, b2->code);
        }
    }
    else
    {
        strcpy(tree->var, tree->token); //value
    }

     if(tree->paramFlag==1 && strcmp(tree->type,"bool")==0){
        tree->code = (char *)realloc(tree->code, strlen(tree->code) + 4*strlen(tree->nextLabel) + 40);
         strcpy(tree->var,freshVar());
        strcat(tree->code,tree->trueLabel);
        strcat(tree->code,":");
        gen(tree,tree->var,NULL,NULL,"1");
        strcat(tree->code,"\tgoto ");
        strcat(tree->code,tree->nextLabel);
        strcat(tree->code,"\n");
        strcat(tree->code,tree->falseLabel);
        strcat(tree->code,":");
        gen(tree,tree->var,NULL,NULL,"0");
        strcat(tree->code,tree->nextLabel);
        strcat(tree->code,":");

    }
}

void addLabelsToTree(node *tree)
{
    node *b1 = tree->left, *b2 = tree->right;

    /////////////////////////// Pre order/////////////////////////////

    if (strcmp(tree->token, "DO-WHILE") == 0)
    {
        tree->trueLabel = freshLabel(); //begin
    }
    else if (strcmp(tree->token, "FOR") == 0)
    {
        tree->trueLabel = freshLabel();
        // if(strcmp(b2->token,"FOR")==0 && b2->oneStmt==1)
        //     b2->label=freshLabel();
    }
    if (tree->left)
        addLabelsToTree(tree->left);

    /////////////////////////// In order////////////////////////////////
    if (strcmp(tree->token, "IF") == 0)
    {
        b1->trueLabel = freshLabel(); // b1=cond
    }
    else if (strcmp(tree->token, "STMT") == 0)
    {
        if (strcmp(b1->token, "IF") == 0 || strcmp(b1->token, "IF-ELSE") == 0 || strcmp(b1->token, "WHILE") == 0
         || strcmp(b1->token, "FOR") == 0 || strcmp(b1->token, "DO-WHILE") == 0
         || (b1->right && strcmp(b1->right->type,"bool")==0 ))
        {
            tree->nextLabel = freshLabel();
        }
    }

    else if (strcmp(tree->token, "IF-ELSE") == 0)
    {
        b1->falseLabel = freshLabel(); //ze bichlal false label
    }
    else if (strcmp(tree->token, "WHILE") == 0)
    {  
        tree->trueLabel = freshLabel(); //begin
        b1->trueLabel = freshLabel();
    }
    else if (strcmp(tree->token, "FOR") == 0)
    {  
       b1->right->left->trueLabel = freshLabel(); //expr
       
    }
    
    if (tree->right)
        addLabelsToTree(tree->right);

    /////////////////////////// Post order////////////////////////////

    if (strcmp(tree->token, "||") == 0 )
    {
        b1->falseLabel = freshLabel();
    }
    else if (strcmp(tree->token, "&&") == 0 )
    {
        b1->trueLabel = freshLabel();
    }

    else if (strcmp(tree->token, "IF") == 0)
    { 
    }

    else if (strcmp(tree->token, "IF-ELSE") == 0)
    {                                  //post if-else
       // b1->falseLabel = freshLabel(); //ze bichlal false label
    }
    else if(strcmp(tree->token,"=")==0){
        if(tree->right && strcmp(tree->right->type,"bool")==0){
            tree->trueLabel=freshLabel();
            tree->falseLabel=freshLabel();
        }
    }
    else if (strcmp(tree->token, "FOR") == 0)
    {  
        //if(strcmp(b2->token,"FOR")==0 && b2->oneStmt==1) $$$
         if(b2->oneStmt==1) 
            b2->label=freshLabel();
    }
    if(tree->paramFlag==1 && strcmp(tree->type,"bool")==0){
        tree->trueLabel=freshLabel();
        tree->falseLabel=freshLabel();
        tree->nextLabel=freshLabel();

    }
}

void secondScan(node *tree)
{

    //calc inherited in preorder:
    calcInherited(tree);

    if (tree->left)
        secondScan(tree->left);
    if (tree->right)
        secondScan(tree->right);

    //code in postorder:
    calcCode(tree);
}

void calcInherited(node *tree)
{
    node *b1 = tree->left, *b2 = tree->right;

    if (strcmp(tree->token, "||") == 0 )
    {
        strcpy(b1->trueLabel, tree->trueLabel);
        strcpy(b2->trueLabel, tree->trueLabel);
        strcpy(b2->falseLabel, tree->falseLabel);
    }
    else if (strcmp(tree->token, "&&") == 0)
    {
        strcpy(b1->falseLabel, tree->falseLabel);
        strcpy(b2->trueLabel, tree->trueLabel);
        strcpy(b2->falseLabel, tree->falseLabel);
    }
    else if (strcmp(tree->token, "!") == 0)
    {
        strcpy(b2->trueLabel, tree->falseLabel);
        strcpy(b2->falseLabel, tree->trueLabel);
    }
    else if (strcmp(tree->token, "IF-ELSE") == 0)
    {
        if(b1->right->oneStmt==1 || b2->left->oneStmt==1)
            oneStmt(tree);
        
        strcpy(b1->nextLabel, tree->nextLabel);
        strcpy(b2->left->nextLabel, tree->nextLabel);
    }
    else if (strcmp(tree->token, "IF") == 0)
    {
        if (tree->parent)
            strcpy(b1->falseLabel, tree->falseLabel);
        else
            strcpy(b1->falseLabel, tree->nextLabel);
        //$$$strcpy(b2->nextLabel, tree->nextLabel);
        if(b2->oneStmt==1)
            oneStmt(tree);
    }
    else if (strcmp(tree->token, "WHILE") == 0)
    {     
        strcpy(b1->falseLabel, tree->falseLabel); //expr
        if(b2->oneStmt==1)
            oneStmt(tree);
    }
    else if (strcmp(tree->token, "DO-WHILE") == 0)
    {
        strcpy(b2->falseLabel, tree->falseLabel);
        strcpy(b2->trueLabel, tree->trueLabel);
    }
    else if (strcmp(tree->token, "FOR") == 0)
    {   
        strcpy(b1->right->left->falseLabel, tree->falseLabel);
        if(b2->oneStmt==1)
            oneStmt(tree);
    }
    else if (strcmp(tree->token, "STMT") == 0)
    {   if(strcmp(b1->token,"=")!=0 ){
        strcpy(b1->falseLabel, tree->nextLabel);}
        strcpy(b1->nextLabel, tree->nextLabel);
    
    }
    else if(strcmp(tree->token, "=") == 0 ){
        if(b2){
            strcpy(b2->trueLabel,tree->trueLabel);
            strcpy(b2->falseLabel,tree->falseLabel);
        }

    }
}

void calcCode(node *tree)
{
    node *b1 = tree->left, *b2 = tree->right;
    if (strcmp(tree->token, "STMT") == 0)
    { //handle stmt code string
        tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(tree->nextLabel) + strlen(b1->code) + 20);
        strcat(tree->code, b1->code);
        if (strcmp(tree->nextLabel, "") != 0)
        {
            strcat(tree->code, tree->left->nextLabel);
            strcat(tree->code, ":");
            if ( (b2 == NULL || (b2 && strcmp(b2->code, "") == 0 ))
             || (b1 && (strcmp(b1->token,"FOR")==0||strcmp(b1->token,"IF")==0 || strcmp(b1->token,"IF-ELSE")==0  )))
                strcat(tree->code, "\n");
        }
        //add right son of stmt to the code string
        if (b2)
        {
            tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(b2->code) + 1);
            strcat(tree->code, b2->code);
        }
    }
    else if (strcmp(tree->token, "IF") == 0)
    {
        calcIf(tree);
    }
    else if (strcmp(tree->token, "WHILE") == 0)
    {
        calcWhile(tree);
    }
    else if (strcmp(tree->token, "DO-WHILE") == 0)
    {
        calcDoWhile(tree);
    }
    else if (strcmp(tree->token, "ELSE") == 0)
    {
        tree->code = (char *)realloc(tree->code, strlen(b1->code) + 20);
        if(strcmp(b1->token,"WHILE")==0 ||
         (strcmp(b1->token,"BLOCK")==0 && b1->left && b1->left->left && strcmp(b1->left->left->token,"WHILE")==0))
            strcat(tree->code, "\n");
        strcat(tree->code, b1->code);
    }

    else if (strcmp(tree->token, "IF-ELSE") == 0)
    {
        tree->code = (char *)realloc(tree->code, strlen(b1->code) + strlen(b2->code) + strlen(b1->nextLabel) + strlen(b1->left->falseLabel) + 20);
        strcat(tree->code, b1->code);
        strcat(tree->code, "\tgoto ");
        strcat(tree->code, b1->nextLabel);
        strcat(tree->code, "\n");
        strcat(tree->code, b1->left->falseLabel);
        strcat(tree->code, ":");
        if(strcmp(b2->token,"WHILE")==0 ||
            (strcmp(b2->token,"BLOCK")==0 && b2->left && b2->left->left && strcmp(b2->left->left->token,"WHILE")==0))
            strcat(tree->code, "\n");
        strcat(tree->code, b2->code);
    }
    else if (strcmp(tree->token, "FOR") == 0)
    {
        calcFor(tree);
    }
    else if (strcmp(tree->token, "FuncCall") == 0)
    {
        calcFuncCall(tree);
    }
    else if (strcmp(tree->token, "BLOCK") == 0 || strcmp(tree->token, "COND") == 0)
    {
        if (b1)
        {
            tree->code = (char *)realloc(tree->code, strlen(b1->code) + 20);
            strcat(tree->code, b1->code);
        }
    }
    else if (strcmp(tree->token, "=") == 0){
        char *temp = (char *)malloc(sizeof(char) + 1);
        if(b2 && strcmp(b2->type,"bool")==0){ //tree right is expr

                strcat(tree->code,b2->code);
                strcat(tree->code,b2->trueLabel);
                strcat(tree->code,":");
                gen(tree,b1->var,NULL,NULL,"1");
                strcat(tree->code,"\tgoto ");
                strcat(tree->code,tree->nextLabel);
                strcat(tree->code,"\n");
                strcat(tree->code,b2->falseLabel);
                strcat(tree->code,":");
                gen(tree,b1->var,NULL,NULL,"0");


            }
        else if(tree->left && (tree->left->type,"string")==0){
            calcVarDec(tree);
        }
        else if(tree->parent && strcmp(tree->parent->token,"STMT")!=0){ //VARDEC
            calcVarDec(tree);
        }


        else{ //ASS
            if (tree->left->left && strcmp(tree->left->left->token, "INDEX") == 0)
            {
                //hanle ass
                strcpy(tree->var, tree->left->var);
                cat2sons(tree);
                strcpy(temp, "*");
                strcat(temp, tree->var);
                gen(tree, temp, tree->token, NULL, tree->right->var);
            }
            else if (strcmp(tree->left->token, "*") == 0)
            {
                strcpy(temp, "*");
                strcat(temp, tree->left->right->token);
                strcpy(tree->var, temp); //var=id token
                cat2sons(tree);
                gen(tree, tree->var, tree->token, NULL, tree->right->var);
            }
            
            else
            {
                strcpy(tree->var, tree->left->token); //var=id token
                cat2sons(tree);
                gen(tree, tree->var, tree->token, NULL, tree->right->var);
            }
        }
    }
    else if(strcmp(tree->token, "BODY") == 0 || strcmp(tree->token, "IGNORE") == 0
     || strcmp(tree->token, "REC_FUNC") == 0){
        cat2sons(tree);
    }
    else if(strcmp(tree->token, "VARDEC") == 0){
        cat2sons(tree->left); // tree->left is TYPE
        cat2sons(tree);
    }
    else if(strcmp(tree->token, "STRING") == 0){
        cat2sons(tree);

    }
    else if(isFUNC(tree->token)){
        calcFunc(tree);
    }
    else if(strcmp(tree->token, "RETURN") == 0){
        node* expr= tree->left;
        tree->code = (char *)realloc(tree->code, strlen(tree->code)+strlen(tree->left->var)+strlen(tree->left->code) + 30);
        if(!(expr->var[0]=='t' || strcmp(expr->type,"ID")==0))
        {
            strcpy(expr->var,freshVar());
            gen(tree,expr->var,NULL,NULL,expr->token);
        }
        strcat(tree->code,tree->left->code);
        strcat(tree->code,"\tReturn ");
        strcat(tree->code,tree->left->var);
        strcat(tree->code,"\n");


    }
    else
    {
        expr_scan2(tree);
    }
}

//////////help funcs for calc code/////////////

void calcIf(node *tree)
{
    node *b1 = tree->left, *b2 = tree->right;
    //if the expr us true/false we want to jump to the true/right label without "if"
    if (strcmp(b1->token, "true") == 0 || strcmp(b1->token, "false") == 0)
    {
        char *label = (char *)malloc(sizeof(char) + 1);
        if (strcmp(b1->token, "true") == 0)
            strcpy(label, b1->trueLabel);
        else
            strcpy(label, b1->falseLabel);
        tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(label) + 20);
        strcat(tree->code, "\tgoto ");
        strcat(tree->code, label);
        strcat(tree->code, "\n");
    }
    else if (strcmp(b1->type, "ID") == 0)
    { //if the if cond contains only one identifier
        strcat(tree->code, b1->code);
        strcat(tree->code, "\tif ");
        strcat(tree->code, b1->var);
        strcat(tree->code, " goto ");
        strcat(tree->code, b1->trueLabel);
        strcat(tree->code, "\n\tgoto ");
        strcat(tree->code, b1->falseLabel);
        strcat(tree->code, "\n");
    }
    //adding the aditional of cond calculation to the code
    tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(b1->code) + strlen(b2->code) + 2 * strlen(b1->trueLabel) + 10);
    strcat(tree->code, b1->code);
    strcat(tree->code, b1->trueLabel);
    strcat(tree->code, ":");
    if(strcmp(b2->token,"WHILE")==0)
        strcat(tree->code, "\n");
    strcat(tree->code, b2->code);
}

void calcWhile(node *tree)
{
    node *b1 = tree->left, *b2 = tree->right;
    tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(b1->code) + strlen(b2->code) + 2 * strlen(b1->trueLabel) + 20);
    strcat(tree->code, tree->trueLabel);
    strcat(tree->code, ":");
    strcat(tree->code, b1->code);
    strcat(tree->code, b1->trueLabel);
    strcat(tree->code, ": ");
    if(strcmp(b2->token,"WHILE")==0)
        strcat(tree->code, "\n");
    strcat(tree->code, b2->code);
    strcat(tree->code, "\tgoto ");
    strcat(tree->code, tree->trueLabel);
    strcat(tree->code, "\n");
}

void calcDoWhile(node *tree)
{
    node *b1 = tree->left, *b2 = tree->right;
    tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(b1->code) + strlen(b2->code) + 2 * strlen(b1->trueLabel) + 20);

    strcat(tree->code, tree->trueLabel); //inside code
    strcat(tree->code, ":");
    strcat(tree->code, b1->code);
    strcat(tree->code, b2->code);
}

void calcFor(node *tree)
{

    node *b1 = tree->left, *b2 = tree->right;
    tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(b1->left->code) + strlen(b1->right->code) +
                 strlen(b1->right->right->code) + strlen(b2->code) + 3 * strlen(b1->trueLabel) + 20);

    strcat(tree->code, b1->left->code); //init
    strcat(tree->code, tree->trueLabel);
    strcat(tree->code, ":");
    strcat(tree->code, b1->right->code);
    strcat(tree->code, b1->right->left->trueLabel); //expr
    strcat(tree->code, ":"); 
    if(strcmp(b2->token,"WHILE")==0 || strcmp(b2->token,"DO-WHILE")==0 || strcmp(b2->token,"DO-WHILE")==0 ||
        (strcmp(b2->token,"BLOCK")==0 && b2->left && b2->left->left &&
         (strcmp(b2->left->left->token,"WHILE")==0)|| strcmp(b2->left->left->token,"DO-WHILE")==0) )
            strcat(tree->code,"\n");
    strcat(tree->code, b2->code);               //body
    // if(b2->oneStmt==1 && strcmp(b2->token,"FOR")==0){$$$
    //     strcat(tree->code,b2->label);
    //     strcat(tree->code, ":");

    // }
    if(b2->oneStmt==1){//$$
        strcat(tree->code,b2->label);
        strcat(tree->code, ":");
    }
    strcat(tree->code, b1->right->right->code); //update
    strcat(tree->code, "\tgoto ");

    strcat(tree->code, tree->trueLabel);
    strcat(tree->code, "\n");

}

void calcFuncCall(node *tree)
{   
    int popSize=0;
    Var* v;
    node* params= tree->right;
    char* funcName=(char*)malloc(sizeof(char)+1);
    char* ret_type=(char*)malloc(sizeof(char)+1);
    strcpy(ret_type,tree->type);
    strcpy(funcName,tree->left->token);    

    Scope* s=getScopeByID(codeTables->scope,0);
    Element* e = findElementInScope(s,funcName);
    Func* f=e->func_element;

    if(f->args_num==0){
        if(strcmp(ret_type,"void")==0){
            strcat(tree->code,"\tLCALL ");
            strcat(tree->code,funcName);
            strcat(tree->code,"\n");
        }
        else{
            strcpy(tree->var,freshVar());
            gen(tree,tree->var,"LCALL ",NULL,funcName);
        }
    }
    else{
        for(int i=f->args_num-1;i>=0;i--){
            v=f->args[i];
            if(strcmp(v->type,"char")==0)
                popSize+=1;
            else
                popSize+=4;
        }
        //params_val
        params=params->left; //=funcExpr (expr or expr ',' expr)
        pushParams(params,tree);
        if(strcmp(ret_type,"void")==0){
            strcat(tree->code,"\tLCALL ");
            strcat(tree->code,funcName);
            strcat(tree->code,"\n");
        }
        else{
            strcpy(tree->var,freshVar());
            gen(tree,tree->var,"LCALL ",NULL,funcName);
        }
        strcat(tree->code,"\tpopParams ");
        char pop_num[20];
        sprintf(pop_num, "%d", popSize);
        strcat(tree->code,pop_num);
        strcat(tree->code,"\n");
        
    }

}

void calcFunc(node* tree){
    node* help=tree->left; //ID
    tree->code = (char *)realloc(tree->code, strlen(tree->code)+strlen(help->token) + 30);
    strcat(tree->code,help->token); //gen label = "funcname:"
    strcat(tree->code,":\n");
    strcat(tree->code,"\tBeginFunc\n");
    if(help->right->left){ // if body is not null
        tree->code = (char *)realloc(tree->code, strlen(tree->code)+strlen(help->right->left->code) + 30);
        strcat(tree->code,help->right->left->code); //copy body code
    }
    strcat(tree->code,"\tEndFunc\n");

}

void pushParams(node* param,node* tree){

    if(strcmp(param->token,",")==0){
        if(param->right)
        pushParams(param->right,tree);
        if(param->left)
            pushParams(param->left,tree);     
    }
    else{  //param=expr
        tree->code = (char *)realloc(tree->code, strlen(tree->code)+strlen(param->var) + strlen(param->code) +30);
        strcat(tree->code,param->code);
        //gen pushparam var
        if(!(param->var[0]=='t' || strcmp(param->type,"ID")==0))
        {
            strcpy(param->var,freshVar());
            if(strcmp(param->type,"bool")==0){

            }
            gen(tree,param->var,NULL,NULL,param->token);
        }
        strcat(tree->code,"\tPushParam ");
        strcat(tree->code,param->var);

        strcat(tree->code,"\n");
       
    }




}

void calcVarDec(node* tree){ //tree is "="
    node* id;
    if(strcmp(tree->parent->type,"ID")==0)
        id=tree->parent;
    else //parent is TYPE , type->left is ID
        id=tree->parent->left;

    tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(tree->left->code)+10);
    strcat(tree->code,tree->left->code); //copy expr code
    gen(tree,id->token,NULL,NULL,tree->left->var); //tree->left is expr
    if(tree->right) // tree->right is nextvar
    {
        tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(tree->right->code)+10);
        strcat(tree->code,tree->right->code);
    }
    
        
    

}

///////////////help funcs//////////////////////

char *freshVar()
{ //return new temp var
    char str_temp[20];
    char *temp = (char *)malloc(sizeof(char) + 1);
    strcpy(temp, "t");
    sprintf(str_temp, "%d", temp_index++);
    strcat(temp, str_temp);
    return temp; // t + index
}
char *freshLabel()
{ //create fresh label
    char str_num[20];
    char *label = (char *)malloc(sizeof(char) + 1);
    strcpy(label, "L");
    sprintf(str_num, "%d", label_num++);
    strcat(label, str_num);
    return label; // L + num
}

void resetLabel()
{
    label_num = 0;
}

void gen(node *tree, char *var, char *op, char *left, char *right)
{ //gen one line of leagal 3ac code

    // tree->code = reSize(tree->code,40);
    tree->code = (char *)realloc(tree->code, strlen(tree->code) + 40);
    strcat(tree->code, "\t");
    strcat(tree->code, var);
    strcat(tree->code, " = ");
    if (left)
        strcat(tree->code, left);

    if (op && strcmp(op, "=") != 0)
        strcat(tree->code, op);

    strcat(tree->code, right);
    strcat(tree->code, "\n");
}

void cat2sons(node *tree)
{ //combine sons code to father code
    if (tree->left)
    {
        tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(tree->left->code) + 1);
        strcat(tree->code, tree->left->code);
    }
    if (tree->right)
    {
        tree->code = (char *)realloc(tree->code, strlen(tree->code) + strlen(tree->right->code) + 1);
        strcat(tree->code, tree->right->code);
    }
}

void printT(node *tree)
{
    
    printf("%s: false= %s,true= %s,next=%s,var=%s,type=%s,label=%s\n", tree->token, tree->falseLabel, tree->trueLabel, tree->nextLabel,tree->var,tree->type,tree->label);
   if(tree->parent)
    printf("parent is : %s\n",tree->parent->token);
   if (tree->left)
        printT(tree->left);
    if (tree->right)
        printT(tree->right);
}

char *reSize(char *ptr, int size)
{
    char *t = (char *)malloc(sizeof(char) + strlen(ptr) + size + 1);
    strcpy(t, ptr);
    free(ptr);
    return t;
}

int oneStmt(node* tree){
    node* son,*son2; //son is one stmt
    int ret=0;
    if(strcmp(tree->token,"WHILE")==0 || strcmp(tree->token,"FOR")==0 || strcmp(tree->token,"IF")==0)
        son=tree->right;
    else if(strcmp(tree->token,"IF-ELSE")==0){
        son=tree->left->right;
        son2=tree->right->left;
    }
    else 
        return ret;

    if(strcmp(son->token,"IF")==0 ){
        strcpy(son->nextLabel,tree->nextLabel);
        ret=1;

        if(strcmp(tree->token,"WHILE")==0){ //$$$
            strcpy(son->nextLabel,tree->trueLabel);
        }
        else if(strcmp(tree->token,"FOR")==0){//$$$
            strcpy(son->nextLabel,son->label);
        }
    }
    else if(strcmp(tree->token,"WHILE")==0 ){
        strcpy(son->nextLabel,tree->nextLabel);
        strcpy(son->falseLabel,tree->trueLabel);
        ret=1;

    }
    else if(strcmp(tree->token,"IF")==0 ){//$$$
        if(strcmp(son->token,"FOR")==0 || strcmp(son->token,"WHILE")==0 ){
            strcpy(son->nextLabel,tree->nextLabel);
            strcpy(son->falseLabel,tree->nextLabel);

        }
    }
    else if(strcmp(tree->token,"FOR")==0 ){
        strcpy(son->nextLabel,tree->nextLabel);
        if(strcmp(son->token,"FOR")==0 )
            strcpy(son->falseLabel,son->label);
        else if(strcmp(son->token,"IF-ELSE")==0){
            strcpy(son->nextLabel,son->label);
        }
        else
            strcpy(son->falseLabel,tree->trueLabel);
        ret=1;
        

    }
     else if(strcmp(tree->token,"IF-ELSE")==0){
        if(son->oneStmt==1){
            strcpy(son->nextLabel,tree->nextLabel);
            strcpy(son->falseLabel,tree->trueLabel);
        ret=1;

        }
        if(son2->oneStmt==1)
        {
            strcpy(son2->nextLabel,tree->nextLabel);
            strcpy(son2->falseLabel,tree->trueLabel);
        ret=1;

        }
    }



    return ret;
}

void writeToFIle(char* TACcode){
    FILE* pfile;
    pfile=fopen("output_3ac.txt","w");
    if(pfile==NULL){
        printf("open file error!\n");
        exit(1);
    }
    fprintf(pfile,"%s",TACcode);
    fclose(pfile);

}