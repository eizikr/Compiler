#pragma once
#include "ScopeID_List.c"

//	Linked list struct definition
typedef struct Names_List {
	char* ID_name;
    ScopeID_List* scope_list;
	struct Names_List* next;
}Names_List;


Names_List * BuildNode(char* n, int scope)
{//	Creat list with 1 organ and returns a pointer
	Names_List *element = (Names_List*)malloc(sizeof(Names_List));	//	Allocate list memory
	element->ID_name = (char*)malloc(sizeof(char)+1);
	strcpy(element->ID_name ,n);
    element->scope_list = addToStartScopeID(element->scope_list,scope);	//	Allocate string memory
	element->next = NULL;
	return element;
}

Names_List* AddScopeToName(Names_List* head, char*n, int scope)
{
    Names_List* temp = head;
    while(temp != NULL)
    {
        if(strcmp(temp->ID_name,n) == 0)
        {
            temp->scope_list = addToStartScopeID(temp->scope_list, scope);
            break;
        }
        else
        {
            temp = temp->next;
        }
        
    }  
    return head;  
}

Names_List* addToStart(Names_List* head,char* n, int scope)
{//	Create a new element and make him the head of the list.
	Names_List* new_elem = BuildNode(n,scope);
	new_elem->next = head;
	return new_elem;
}

 void FreeList(Names_List* head)
{//	Free allocated memory of the list
	Names_List *temp;
    FreeListScopeID(temp->scope_list);
	while (head != NULL)
	{	//	Free every element
		temp = head;
		head = head->next;
		free(temp);
	}

}

Names_List* DeleteFirstElement(Names_List* head, int n)
{//	Delete one element fromt the list
	if (head == NULL){	//	Emptey list case
		printf("the list is already empty\n");
		return head;
	}
    Names_List* temp=head;
	head= head->next;

    free(temp);
	return head;
}

Names_List* deleteName(Names_List* list, Names_List* element){
    if(list==element)
        return element->next;
    
    Names_List *p=NULL,*temp=list;
    while(temp->next!=NULL){
        if(temp->next==element){
            p=element;
            break;
        }
    }
    p=element->next;
    return list;
    

}

Names_List* deleteScopeNames(Names_List* list,int scope){
    Names_List* temp=list;
    
    while(temp!=NULL){

        temp->scope_list=deleteScopeID(temp->scope_list,scope);
        if(temp->scope_list==NULL){
            list = deleteName(list,temp);
        }
        temp=temp->next; //next name
        
    }
    return list;
}

void printNamesList(Names_List* list){
    Names_List* temp=list;
    while(temp!=NULL){
        printf("name is: %s\nscopes list:",temp->ID_name);
        printScopeIDList(temp->scope_list);
        temp=temp->next;

    }
}

int isNameExists(Names_List* head,char* name){
    Names_List* temp=head;
    while(temp!=NULL){
        if(strcmp(temp->ID_name,name)==0){
            return temp->scope_list->scopeID; // return the scope id
        }
        temp=temp->next;
    }
    return -1; // if not exists

}