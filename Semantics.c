#include "Stack.c"
#include "Bind_HT.c"
#include "astFuncs.c"

typedef struct Code{
    Bind_HT* ht;
    Scope* scope; //scope next is the previous environment
}Code;

Code* buildCode(Code* c){
    c=(Code*)malloc(sizeof(Code));
    c->ht=initTable();
    c->scope=buildScope();
    return c;
}
Code* pop(Code* code);

Code* codeTables;

void semanticsScan(Code* code,node* tree);
void vardecSemantics(Code* code,node* tree);
void strdecSemantics(Code* code,node* tree);
void checkAssignment(Code* code,node* tree);
Func* funcSemantics(Code*code,node* tree);
void printError(int err,char* info,int line);
void exprSemantics(Code*code,node* tree);
void checkFuncCall(Code* code,node* tree);
Func* getFunc(Code* c,node* t);
int isBinaryOperator(char* operator);
char* typeID(Code* c, node* id);
char* getType(Code*,node*);
void condCheck(Code* c,node* cond);
char* extractType(char* s);
//checks for expr semantics:
node* realOrIntCheck(Code* c,node* tree);
node* realOrIntCheckEqualBool(Code* c,node* tree);
void checkOperator_EqlEql_NotEql(Code* c,node* tree);
void checkAddr(Code*,node*);
void checkReturnExpr(Code* code,node* tree, Func* f);
void checkMainExists(Code* code);
int isPtr(char*,char*);
void checkDereference(Code* code,node* tree);



Code* pop(Code* code){
    int scopeid=code->scope->scopeID;
    code->scope=popScope(code->scope);
    code->ht=deleteScopeFromHT(code->ht,scopeid);
}


void checkMainExists(Code* code){
    if(isNameInBindHT(code->ht,"main",0)==0)
        printError(MAINFUNC_ERROR,"every program must have main function",0);
}
void semanticsScan(Code* code,node* tree){
    if(strcmp(tree->token,"BLOCK")==0 || isFUNC(tree->token) ){


        Scope* new_scope=buildScope();
        if(isFUNC(tree->token))
        {   
            int line=tree->left->left->line;
            Func* f = funcSemantics(code,tree);
            new_scope->f=f;
            char* name=(char*)malloc(sizeof(char)+1);
            for(int i=0;i<f->args_num;i++)
            {   strcpy(name, f->args[i]->name);
                if(isNameInBindHT(code->ht,name,new_scope->scopeID))
                    printError(DEC_ERROR,name,line);

                new_scope=addElementToScope(new_scope,buildElement(f->args[i],NULL));   
                code->ht=addName(code->ht,name,new_scope->scopeID);
            }
            tree=tree->left->right; //id, tree->rugth=body
        }
        else{
            new_scope->f=code->scope->f; //if enter block ->take the last func
        }
        code->scope=pushScope(code->scope,new_scope);
        if(tree->left ){
            semanticsScan(code,tree->left);}
        if(tree->right  ){
            semanticsScan(code,tree->right);} 
           
        
         code = pop(code);
       

        
    }
    else if(strcmp(tree->token,"RETURN") == 0){
        checkReturnExpr(code,tree->left,code->scope->f);
    }
    else if(strcmp(tree->token,"VARDEC") == 0){
        vardecSemantics(code,tree);
    }
    else if(strcmp(tree->token,"STRING") == 0){

        strdecSemantics(code,tree);
    }
    else if((strcmp(tree->token,"IGNORE")==0) || (strcmp(tree->token,"DEC") == 0) || (strcmp(tree->token,"STMT")==0) || (strcmp(tree->token,"BODY")==0)
            || (strcmp(tree->token,"REC_FUNC")==0)){
        if(tree->left)
            semanticsScan(code,tree->left);
        if(tree->right)
            semanticsScan(code,tree->right);  
    }
    else if(strcmp(tree->token,"FuncCall")==0){
       checkFuncCall(code,tree);
     }
    else if(strcmp(tree->token,"INDEX")==0){ 
        node* index;
        if(tree->right)
            index=tree->right;
        else
            index=tree->left;
         exprSemantics(code,index);
        if(strcmp(getType(code, index), "int") != 0){
            printError(INDEX_MISSTYPE,"",tree->line);
        }
    }
    else if(strcmp(tree->token,"IF")==0 || strcmp(tree->token,"WHILE") == 0 || strcmp(tree->token,"COND")==0) //cond=for
    {  
        condCheck(code,tree->left);
        if(tree->right)
            semanticsScan(code, tree->right);
    }
    else if(strcmp(tree->token,"FOR")==0){
        semanticsScan(code,tree->left->left); //init 
       condCheck(code,tree->left->right->left); // cond expr
        semanticsScan(code,tree->left->right->right); //update
        if(strcmp(tree->left->right->right->type,"int")!=0){
            printError(TYPE_MISS,"int for loop - update variable must be int",tree->line);
        }
        semanticsScan(code,tree->right); //statement

    }
    else if(strcmp(tree->token,"DO-WHILE") == 0)
    {   condCheck(code,tree->right);

        if(tree->left)
            semanticsScan(code, tree->left);
    }
    else if(strcmp(tree->token,"=") == 0){
        checkAssignment(code,tree);
    }
 
    else{
        exprSemantics(code,tree);
        // if(tree->left)
        //     semanticsScan(code,tree->left);
        // if(tree->right)
        //     semanticsScan(code,tree->right);
    }
}


void  checkAssignment(Code* code,node* tree){
    int flag=0;
    node* mul=tree->left; 
    exprSemantics(code,tree->left);
    exprSemantics(code,tree->right);
    if(strcmp("*",tree->left->token)==0)// *id=exp
    {    tree->left=tree->left->right; //=id
         flag=1;
    }
    char* err=(char*)malloc(sizeof(char)+40);
    strcpy(err,"you can not assign type: ");

    char* val=(char*)malloc(sizeof(char)+1);
    char* type_l=(char*)malloc(sizeof(char)+1);
    char* type_r=(char*)malloc(sizeof(char)+1);
    
    //get element:
     Scope* s;
    Element* e;
    char* type=(char*)malloc(sizeof(char)+1);
    int scope = getScope(code->ht,tree->left->token); //get scope id by name
    if(scope == -1)
    {
        printError(UNDECLERED_ID, tree->left->token,tree->line);
    }
    s=getScopeByID(code->scope,scope);
    e = findElementInScope(s,tree->left->token);
    
    strcpy(type_r,getType(code,tree->right));
    strcpy(e->var_element->value,"init");
    if(flag){
        strcpy(type_l,getType(code,mul));
    }
    else
        strcpy(type_l,getType(code,tree->left));

    if(strcmp(type_l,type_r)!=0 ){
        if(isPtr(type_l,type_r)==1){
            strcpy(val,"0");
        }

        else if(!(strcmp(type_l,"real")==0 && strcmp(type_r,"int")==0)){
            strcat(err,type_r);
            strcat(err," in type: ");
            strcat(err, type_l);
            printError(TYPE_MISS,err,tree->line);
            
        }
    }
    else{
        strcpy(val,tree->right->token);
    }
    strcpy(tree->type,type_l);
    strcpy(e->var_element->value,val);

}
void condCheck(Code* code,node* cond){
   node* help= cond;
    exprSemantics(code,help);
        if(strcmp(getType(code, help), "bool") != 0)
            printError(COND_MISSTYPE,"",help->line);
    

}
void strdecSemantics(Code* code,node* tree){
    char* len=(char*)malloc(sizeof(char)+1);
    int isfirst=1;
    char* name,*val;
    char* type="string";
    char* type_r; //needs to be strval
    node* nextStr;
    Var* v;

    while(tree!=NULL){
        val="none";
        if(isfirst){
            name=tree->left->token; //id
            strcpy(len,tree->left->left->token);//intval
        }
        else{
            name=tree->token;
            len=tree->left->token;
        }
        
       tree=tree->right; //next strarr
        if(tree && strcmp(tree->token,"=")==0){
            val=tree->left->token;
            tree=tree->right;
        }
        v = buildVar(name,val,type,len);    // build string

        if(isNameInBindHT(code->ht,name,code->scope->scopeID))
            printError(DEC_ERROR,name,tree->line);

        code->scope=addElementToScope(code->scope,buildElement(v,NULL));   
        code->ht=addName(code->ht,name,code->scope->scopeID);

        isfirst=0;

    }
}

void vardecSemantics(Code* code,node* tree){

    tree=tree->left; // tree = TYPE
    
    char* val=(char*)malloc(sizeof(char)),*name=(char*)malloc(sizeof(char)),*type=(char*)malloc(sizeof(char)),*err=(char*)malloc(sizeof(char));
    err=(char*)malloc(sizeof(char));
    val=(char*)malloc(sizeof(char));
    strcpy(type,tree->token);
    Var* v;
    char* type_r=(char*)malloc(sizeof(char));
    node* help,*var;
    int line;
    //the first var:
    var= tree->left;
    strcpy(val,"none");
    strcpy(name,var->token); // name = ID 
    line=var->line;
    
    tree = tree->right; // tree is NEXTVAR
    if(tree!=NULL){ 
        
        if(strcmp(tree->token,"=")==0){  // IF NEXTVAR is ASSIGMENT
            help = tree->left; // EXPR

            exprSemantics(code,help); // UPDATE TYPE 
            type_r=getType(code,tree->left);
            if(isPtr(type,type_r)==1){
                strcpy(val,"0");
            }
            else if(strcmp(type_r,type) != 0)
            {
                if(!(strcmp(type,"real")==0 && strcmp(type_r,"int")==0)){
                    err=strcat(name," is Type:");
                    err=strcat(err,type);
                    err=strcat(err,", You cant assign experssion from type: ");
                    err=strcat(err,type_r);
                    printError(TYPE_MISS,err,tree->line);
                }
            }
            else
                strcpy(val,tree->left->token); // MAYBE TO ADD A FUNC THAT RETURN VALUE OF EXPR
            tree=tree->right;            
        }

    }
    
    
    v = buildVar(name,val,type,"0");    // BUILD FIRST VARIABLE

    if(isNameInBindHT(code->ht,name,code->scope->scopeID)){
        printBindHT(code->ht);
        printError(DEC_ERROR,name,line);
    }

    code->scope=addElementToScope(code->scope,buildElement(v,NULL));   
    code->ht=addName(code->ht,name,code->scope->scopeID);

    //next var(id)
    while(tree!=NULL){
        strcpy(name,tree->token);
        line = tree->line;
        strcpy(val,"none");

        if(tree->left!=NULL && strcmp(tree->left->token,"=")==0){
            help = tree->left->left; // EXPR
            exprSemantics(code,help); // UPDATE TYPE 
            strcpy(type_r, getType(code,tree->left->left));
            if(strcmp(type_r,type) != 0){
                err=strcat(name," is Type:");
                err=strcat(err,type);
                err=strcat(err,", You cant assign experssion from type: ");
                err=strcat(err,type_r);
                printError(TYPE_MISS,err,line);
            }
            else
            {
                strcpy(val,tree->left->left->token);// REMEMBER TO ADD A FUNC THAT RETURN VALUE OF EXPR
                tree=tree->left->right;
            }
        }
        else
            tree=tree->left; // NEXTVAR
        

        v=buildVar(name,val,type,"0");

        if(isNameInBindHT(code->ht,name,code->scope->scopeID))
            printError(DEC_ERROR,name,line);
        code->scope=addElementToScope(code->scope,buildElement(v,NULL));
        code->ht=addName(code->ht,name,code->scope->scopeID);
    } 
}
int isPtr(char* type_l,char* type_r){
    if(strcmp(type_r,"NULL")==0){
        if(strcmp(type_l,"char*")==0 || strcmp(type_l,"int*")==0 || strcmp(type_l,"real*")==0 ){
            return 1;
        }
    }
    return 0;
}

Func* funcSemantics(Code*code,node* tree){
    char* ret_type = extractType(tree->token); //func: type
    tree = tree->left; //ID
    int line=tree->left->line;

    char* name = tree->token,*type_Arg = (char*)malloc(sizeof(char)+1);
    int numOfArgs = 0,argsIndex=0;
    Var** args = NULL;
    node* temp = tree->left->left;        //indicate args node 

    while(temp != NULL) //count args
    {
        if(strcmp(temp->type,"ID")==0)
            numOfArgs++;
        temp=temp->left;
        
    }

    temp = tree->left->left;

    args = (Var**)malloc(sizeof(Var*)*numOfArgs);
    
    while(temp != NULL)//builds args
    {

           
        if(strcmp(temp->type,"ID") ==0 ){ 
            args[argsIndex++] = buildVar(temp->token,"param",type_Arg,"0");
           
            
        }
        else if(strcmp(temp->token,"nextType")!=0) //NODE IS TYPE
        {
             strcpy(type_Arg,temp->token);
        }
        

        temp=temp->left;
    }

    //build func
    Func* f= buildFunc(name,ret_type,numOfArgs,args);
    //add func to scope
    if(isNameInBindHT(code->ht,name,code->scope->scopeID))
        printError(FUNCDEC_ERROR,name,tree->line);

    if(strcmp(f->name,"main")==0){
        if(code->scope->scopeID!=0){
            printError(MAINFUNC_ERROR,"main must be declered in the global scope",line);
        }
        if(f->args_num!=0){
            printError(MAINFUNC_ERROR,"main function cant get arguments",line);
        }
        if(strcmp(f->return_type,"void")!=0){
            printError(MAINFUNC_ERROR,"return type of main function must be void",line);
        }

    }

    code->scope=addElementToScope(code->scope,buildElement(NULL,f));   
    code->ht=addName(code->ht,name,code->scope->scopeID);


    return f;
}
void checkReturnExpr(Code* code,node* tree, Func* f){
    //check return statement:


    exprSemantics(code,tree); //expr of return

    if(strcmp(getType(code,tree),f->return_type)!=0)
        printError(RETURN_TYPE,f->return_type,tree->line);
        

}
char* extractType(char* s)
{
    char* ret_type=(char*)malloc(sizeof(char)*6);
    for(int i=6,j=0;i<=strlen(s);i++,j++)
        ret_type[j]=s[i];
    if(strcmp("VOID",ret_type)==0)
        strcpy(ret_type,"void");
    return ret_type;
}
void checkFuncCall(Code* code,node* tree){
    
   char* type=(char*)malloc(sizeof(char)+1);
   char* err=(char*)malloc(sizeof(char)+40);
   strcpy(err,tree->left->token);
   int index=0;
   int count=0;
   node* help;
   Func* f= getFunc(code,tree->left);

   if(strcmp(tree->right->token,"PARAMS_NONE")==0 && f->args_num!=0){
       strcat(err," gets too few arguments");
       printError(TYPE_MISS,err,tree->line);
   }
   else if(strcmp(tree->right->token,"PARAMS_VAL")==0 && f->args_num==0){
       strcat(err," gets too many arguments");
       printError(TYPE_MISS,err,tree->line);
   }
   else if(strcmp(tree->right->token,"PARAMS_VAL")==0){
 
       help= tree->right->left; // expr || expr , expr
       while(help!=NULL){

           if(index>=f->args_num){
                strcat(err," gets too many arguments");
                printError(TYPE_MISS,err,tree->line);
           }

           if(strcmp(help->token,",")!=0){
               
               exprSemantics(code,help);
               
                if(isPtr(f->args[index]->type,getType(code,help))==1){
                   strcpy(f->args[index]->value,"0");
               }
                else if(strcmp(getType(code,help),f->args[index]->type)!=0){
                   strcat(err," - argument type missmatch");
                   printError(TYPE_MISS,err,help->line);
               }

              help=NULL;
           }
           else{ //help= ","
               exprSemantics(code,help->left);
               if(isPtr(f->args[index]->type,getType(code,help->left))==1){
                   strcmp(f->args[index]->value,"0");
               }
               else if(strcmp(getType(code,help->left),f->args[index]->type)!=0){
                   strcat(err," - argument type missmatch");
                   printError(TYPE_MISS,err,help->left->line);
               }

                help=help->right;
               
           }
           index++;
         

       }
       if(index!=f->args_num){
            strcat(err," gets too few arguments");
            printError(TYPE_MISS,err,tree->line);
       }
   }

    addType(tree,f->return_type);


}

Func* getFunc(Code* c,node* name){
    Scope* s;
    Element* e;
    char* type=(char*)malloc(sizeof(char)+1);
    int scope = getScope(c->ht,name-> token); //get scope id by name
    char* err=(char*)malloc(sizeof(char)+40);
    if(scope == -1)
    {
        printError(UNDECLERED_ID, name->token,name->line);
    }
    s=getScopeByID(c->scope,scope);
    e = findElementInScope(s,name->token);
    if(e->isVar){
        strcpy(err,name->token);
       strcat(err," identifier can not be used as a function");
       printError(TYPE_MISS,err,name->line);
    }
    return e->func_element;

}


void checkDereference(Code* code,node* tree){//tree=*,null,exp
    char* type=(char*)malloc(sizeof(char)+1);
    node* temp1,temp2;
    if(strcmp(tree->right->type,"ID")==0){ //*id
        strcpy(type,getType(code,tree->right));
        if(strcmp(type,"char*")==0) 
            strcpy(tree->type,"char");
        else if(strcmp(type,"real*")==0)
            strcpy(tree->type,"real");
        else if(strcmp(type,"int*")==0)
            strcpy(tree->type,"int");
        else{
            printError(TYPE_MISS,"you can not dereference non pointer type ",tree->line);
        }
    }
    else{ //*(id +/- expr)
        temp1=tree->right; //=expr
        while(temp1->left->left!=NULL){ //until temp1=operator next the id
            temp1=temp1->left;
        }

        if(strcmp(temp1->token,"+")==0 || strcmp(temp1->token,"-")==0 ){
            if(strcmp(temp1->left->type,"ID")==0){
                strcpy(type,getType(code,temp1->left));
                if(strcmp(type,"char*")==0)
                    strcpy(tree->type,"char");
                else{ 
                        printError(TYPE_MISS,"dereference pointer expretion must start with char pointer identifier ",tree->line);
                    }

                strcpy(temp1->left->type,"int");
                exprSemantics(code,tree->right); //exp must be int
                if(strcmp(getType(code,tree->right),"int")!=0){
                    printError(TYPE_MISS,"promoting pointer in dereference expretion must be int expertion ",tree->line);
                }
            }
            else{
                printError(TYPE_MISS,"dereference pointer expretion must start with char pointer identifier ",tree->line);    
            } 
        }
        else{
            printError(TYPE_MISS,"you can use only '+' or '-' operator in dereference pointer expretion ",tree->line);
        }
        strcpy(temp1->left->type,"ID");
    }
    }

void exprSemantics(Code*code,node* tree){


   // char *type;
    if(strcmp(tree->token,"FuncCall")==0){
        semanticsScan(code,tree);
    }
    else if(tree->left == NULL && tree->right && strcmp(tree->token,"*")==0) //node=operator,null,exp
            checkDereference(code,tree);
    else{
        if(tree->left)
            exprSemantics(code, tree->left);
        if(tree->right)
            exprSemantics(code, tree->right);
        

        if(strcmp(tree->token,"INDEX")==0)
            semanticsScan(code,tree);

        
        if(tree->left == NULL && tree->right){ //node=operator,null,exp
    
            if(strcmp(tree->token,"!")==0)//operator:! ,the expr must be bool
            {
                if(strcmp(getType(code,tree->right),"bool")==0){
                    tree->type="bool";
                }
                else{
                    printError(TYPE_MISS,"operator ! must be applied on bool exp",tree->line);
                }
            }
            else if(strcmp(tree->token,"STRLEN:")==0){
                if(strcmp(getType(code, tree->right), "string") != 0){
                    printError(STRLEN_MISSTYPE,tree->token,tree->line);
                }
                else{
                    tree->type="int";
                }
            }
            else if(strcmp(tree->token,"-")==0 || strcmp(tree->token,"+")==0)
            {
                strcpy(tree->type, getType(code,tree->right));
            }

            else if(strcmp(tree->token,"&")==0){
                checkAddr(code,tree);

            }
            // else if(strcmp(tree->token,"*")==0){

            //     type=getType(code,tree->right);

            // }


        }
        else if(tree->left && tree->right){//node=exp,operator,exp
            //operator:+,-,*,/
            if(strcmp(tree->token,"+")==0 
                ||strcmp(tree->token,"-")==0 
                || strcmp(tree->token,"*")==0 
                || strcmp(tree->token,"/")==0)
            {
                tree=realOrIntCheck(code,tree);  
            } 
            //operator: >,<,>=,<=
            else if(strcmp(tree->token,">")==0 
                || strcmp(tree->token,"<")==0 
                || strcmp(tree->token,">=")==0 
                || strcmp(tree->token,"<=")==0 )
            {
                tree=realOrIntCheckEqualBool(code,tree);
            }
            //operator: ==,!=
            else if(strcmp(tree->token,"==")==0 
                || strcmp(tree->token,"!=")==0)
            {
                checkOperator_EqlEql_NotEql(code,tree);    
            }
            // Operator: ||, &&
            else if(strcmp(tree->token,"&&")==0 
                || strcmp(tree->token,"||")==0)
            {
                // Both expr must be bool
                if(strcmp(getType(code,tree->left),"bool")==0 
                    && strcmp(getType(code,tree->right),"bool")==0)
                {
                    tree->type="bool";
                }
                else{
                    char* e=(char*)malloc(sizeof(char));
                    e=tree->token;
                    strcat(e," must be applied only on bool expretions");
                    printError(TYPE_MISS,e,tree->line);
                }
            }
        }
    }
}

void checkAddr(Code* code,node* n){
    
    
    char* type=getType(code,n->right);

    char* err=(char*)malloc(sizeof(char) + 40);
    if(strcmp(type,"string") == 0){
        if(n->right == NULL || n->left==NULL){
            printError(TYPE_MISS,"operator & cant be applied on string without index",n->line);            
        }
        exprSemantics(code, n->right);
          
        strcpy(type, "char");
    }
    else if(! (strcmp(type,"real")==0 || strcmp(type,"int")==0 || strcmp(type,"char")==0) ){

        strcpy(err,"operator & cant be applied on ");
        strcat(err,type);
        printError(TYPE_MISS,err,n->line);

    }
    strcpy(n->type,type);
    strcat(n->type,"*");
}

void checkOperator_EqlEql_NotEql(Code* c,node* tree)    //Tree=exp,operator,exp and operator is: ==,!=
{  

     //the type of both exp must be eql and not string
    if(strcmp(getType(c,tree->left),getType(c,tree->right))==0 && strcmp(getType(c,tree->left),"string")!=0)
    {   
            tree->type="bool";
    }
    else if(isPtr(getType(c,tree->left),getType(c,tree->right))==1 || isPtr(getType(c,tree->right),getType(c,tree->left))==1)
    {
            tree->type="bool";
    }
    else
    {
        char* e=(char*)malloc(sizeof(char));
        e=tree->token;
        strcat(e," must be applied on the same types");
        printError(TYPE_MISS,e,tree->line);
    }
    return;
}

node* realOrIntCheck(Code* c,node* tree)//tree=exp,operator,exp and operator=+,*,/,-
{   
    char* type_r=getType(c,tree->right);
    char* type_l=getType(c,tree->left);

    //exp must be real or int . if both are int the tree is int. if one of them is real then the tree is real.
    if((strcmp(type_l,"real")==0 && strcmp(type_r,"real")==0 )
                || (strcmp(type_l,"real")==0 && strcmp(type_r,"int")==0)
                || (strcmp(type_l,"int")==0 && strcmp(type_r,"real")==0)){
                    tree->type="real";
    }
    else if(strcmp(type_l,"int")==0 && strcmp(type_r,"int")==0  )
    {
            tree->type="int";
    }
            
    else{
        char* e=(char*)malloc(sizeof(char));
        e=tree->token;
        printf("left- token %s type %s, left- token %s type %s\n",tree->left->token,type_l,tree->right->token,type_r);

        strcat(e," must be applied on real/int");
        printError(TYPE_MISS,e,tree->line);
    }
    return tree;
}

node* realOrIntCheckEqualBool(Code* c,node* tree)//tree=exp,operator,exp and operator=<,>,>=,<=
{   //exp must be real or int
    char* type_r=getType(c,tree->right);
    char* type_l=getType(c,tree->left);
    
    if((strcmp(type_l,"real")==0 || strcmp(type_l,"int")==0 ) 
    && (strcmp(type_r,"real")==0 || strcmp(type_r,"int")==0 ))
    {
                tree->type="bool";
    }
    else
    {
        char* e=(char*)malloc(sizeof(char));
        e=tree->token;
        strcat(e," must be applied on real/int");
        printError(TYPE_MISS,e,tree->line);
    }
    return tree;
}

char* typeID(Code* c, node* id)
{
    Scope* s;
    Element* e;
    char* type=(char*)malloc(sizeof(char)+1);
    int scope = getScope(c->ht,id-> token); //get scope id by name
    if(scope == -1)
    {
        printError(UNDECLERED_ID, id->token,id->line);
    }
    s=getScopeByID(c->scope,scope);
    e = findElementInScope(s,id->token);
    if(e->isVar){
        // if(strcmp(e->var_element->value,"none")==0)
        //     printError(UNINITIAL_ID,id->token,id->line);
        strcpy(type,e->var_element->type);
    }
    else{
        strcpy(type,e->func_element->return_type);
    }
    return type;
}

char* getType(Code* c ,node* n){
    char* type=(char*)malloc(sizeof(char)+1);
    if(strcmp(n->type,"ID")==0){
        strcpy(type,typeID(c,n));
        if( (n->right && strcmp(n->right->token,"INDEX")==0) || (n->left && strcmp(n->left->token,"INDEX")==0)) {
            if(strcmp(type,"string")==0){
                strcpy(type,"char");
            }
            else{
                printError(STR_INDEX,"indexing can be applied only on string variable",n->line);
            }
        }
    }
    else{
        strcpy(type,n->type);
    }
    return type;
}

void printError(int err,char* info,int line){
    if(line!=0)
        printf("\033[0;31mSEMANTIC ERROR in line %d: \033[0m",line);
    else
        printf("\033[0;31mSEMANTIC ERROR: \033[0m");

    switch (err)
    {
    case DEC_ERROR:
        printf("variable '%s' is already declered\n",info);
        break;
    case FUNCDEC_ERROR:
        printf("function '%s' is already declered\n",info);
        break;
    case TYPE_MISS:
        printf("%s\n",info);
        break;
    case UNDECLERED_ID:
        printf("'%s' Is undeclered\n",info);
        break;
    case COND_MISSTYPE:
        printf("cond must be bool expr\n");
        break;
    case INDEX_MISSTYPE:
        printf("index must be an integer\n");
        break;
    case STRLEN_MISSTYPE:
        printf("strlen must be applied on string, and '%s' is not a string\n",info);
        break;
    case STR_INDEX:
        printf("%s\n",info);
        break;
    case UNINITIAL_ID:
        printf("%s must be initalized before use\n",info);
        break;
    case RETURN_TYPE:
        printf("return type must be %s\n",info);
        break;
    case MAINFUNC_ERROR:
        printf("%s\n",info);
        break;
    default:
        break;
    }
    exit(1);
}