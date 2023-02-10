#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIZE_ELEMENT 50


int scopeID=0;

typedef struct variable{
    char* name;
    char* value;
    char* type;
    char* len;
}Var;

typedef struct function{
    char* name;
    char* return_type;
    int args_num;
    Var** args;
}Func;

typedef struct Element{
    int isVar;
    Var* var_element;
    Func* func_element;
    struct Element* next;
    
}Element;

typedef struct Scope{ //linked list of hash table =STACK
    int scopeID;
    Func* f;
    Element** elements; //arr 
    struct Scope* next;

}Scope;

void printVar(Var* v){
    printf("name: %s, type: %s, value: %s\n",v->name,v->type,v->value);
}

void printFuncElement(Func* f){
    printf("func name: %s ,return type: %s\n",f->name,f->return_type);
    if(f->args_num==0){
        printf("args list: NONE\n");
    }
    else{
         printf("args list:\n");
         for(int i=0;i<f->args_num;i++)
         {
             printVar(f->args[i]);
         }
    }
}


void printElement(Element* e){
    if(e->isVar)
    {
        printf("element is var: \n");
        printVar(e->var_element);
    }
    else{
        printf("element is func: \n");
        printFuncElement(e->func_element);
    }
}

void printElementsList(Element* head){//print linked list of elements
    Element* temp=head;
    while(temp!=NULL){
        printElement(temp);
        temp=temp->next;
    }
}
void printScope(Scope* s){
    printf("Scope id: %d\nelements:\n",s->scopeID);
    for(int i=0;i<SIZE_ELEMENT;i++){
        if(s->elements[i]!=NULL){
            printElementsList(s->elements[i]);
        }
    }
    if(s->f){
        printf("FUNC OF SCOPE:\n");
        printFuncElement(s->f);
    }


}

void printStack(Scope* head)//print linked list of scopes
{
    Scope* temp=head;
    while(temp!=NULL){
        printScope(temp);
        temp=temp->next;
    }


}

Scope* buildScope(){
    Scope* s=(Scope*)malloc(sizeof(Scope));
    s->scopeID=scopeID++;
    s->next=NULL;
    s->elements=(Element**)malloc(sizeof(Element*)*SIZE_ELEMENT);
    s->f=NULL;
    return s;
}


int hashFunc(char* n){

    int sum=0;
    for(int i=0;i<strlen(n);i++){
        sum+=n[i]-'0';
    }
    return sum % SIZE_ELEMENT;
}
Element* addElement(Element* head,Element* e){
    e->next=head;
    return e;
}

Scope* addElementToScope(Scope* s,Element* e){
    char* name;
    if(e->isVar)
        name=e->var_element->name;
    else
        name=e->func_element->name;
    int index=hashFunc(name);
    s->elements[index]=addElement(s->elements[index],e);
    return s;
}
//add free
Scope* popScope(Scope* head){//remember-remove the binds from the main ht
    if(head->scopeID==0){
        printf("you cant pop the global scope\n");
        return head;
    }
    scopeID--;
    return head->next;
}

Scope* pushScope(Scope* head,Scope* new_scope){//remember-add the binds from the main ht
    new_scope->next=head;
    return new_scope;
}

Var* buildVar(char* name,char* value,char* type,char* len){
    Var *var = (Var*)malloc(sizeof(Var));
	var->name = (char*)malloc(sizeof(char)+1);
	strcpy(var->name ,name);
    var->value = (char*)malloc(sizeof(char)+1);
	strcpy(var->value ,value);
    var->type = (char*)malloc(sizeof(char)+1);
	strcpy(var->type ,type);
    var->len=(char*)malloc(sizeof(char)+1);
    strcpy(var->len,len);

    return var;
}

Func* buildFunc(char* name,char* return_type,int args_num,Var** args){
        Func* f=(Func*)malloc(sizeof(Func));
        f->name=(char*)malloc(sizeof(char)+1);
        strcpy(f->name ,name);
        f->return_type=(char*)malloc(sizeof(char)+1);
        strcpy(f->return_type ,return_type);
        f->args_num=args_num;
        f->args=args;
        return f;
}

Element* buildElement(Var* var_element,Func* func_element){
    Element* e=(Element*)malloc(sizeof(Element));
    if(var_element!=NULL){
        e->isVar=1;

    }
    else{
        e->isVar=0;
    }
    e->func_element=func_element;
    e->var_element=var_element;
    e->next=NULL;
    return e;
}

Scope* getScopeByID(Scope* head,int id){
    Scope* temp=head;
    while(temp!=NULL){

        if(temp->scopeID==id)
            return temp;
        temp=temp->next;
    }
    printf("ERROR- scope with id=%d does not exists!\n",id);
    return NULL;
}

Element* findElementInScope(Scope* s, char* name)
{
    int index = hashFunc(name);
    Element* tmp = s->elements[index];
    char* n;
    while(tmp != NULL)
    {
        if(tmp->isVar)
            n=tmp->var_element->name;
        else
            n=tmp->func_element->name;
        if(strcmp(name,n) == 0)
            break;
        tmp = tmp->next;
    }
    return tmp;
}