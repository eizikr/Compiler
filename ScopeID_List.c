#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//	Linked list struct definition
typedef struct ScopeID_List {
	int scopeID;
	struct ScopeID_List* next;
}ScopeID_List;


ScopeID_List * BuildNodeScopeID(int n)
{//	Creat list with 1 organ and returns a pointer
	ScopeID_List *element = (ScopeID_List*)malloc(sizeof(ScopeID_List));	//	Allocate list memory
	element->scopeID = n;	//	Allocate string memory
	element->next = NULL;
	return element;
}

ScopeID_List* addToStartScopeID(ScopeID_List* head, int n)
{//	Create a new element and make him the head of the list.
	if(head!=NULL && head->scopeID ==n)
		return head;
	ScopeID_List* new_elem = BuildNodeScopeID(n);
	new_elem->next = head;
	return new_elem;
	
}

void FreeListScopeID(ScopeID_List* head)
{//	Free allocated memory of the list
	ScopeID_List *temp;
	while (head != NULL)
	{	//	Free every element
		temp = head;
		head = head->next;
		free(temp);
	}

}

ScopeID_List* DeleteFirstElementScopeID(ScopeID_List* head, int n)
{//	Delete one element fromt the list
	if (head == NULL){	//	Emptey list case
		printf("the list is already empty\n");
		return head;
	}
	ScopeID_List* temp=head;
	head= head->next;
	free(temp);
	return head;
}


ScopeID_List* deleteScopeID(ScopeID_List* list,int scope){
	ScopeID_List* temp=list;
    
    if(temp->scopeID==scope){
		return temp->next;
	}
	
	return temp;
}

void printScopeIDList(ScopeID_List* list){
	ScopeID_List* temp=list;
	while(temp!=NULL){
		printf("%d ",temp->scopeID);
		temp=temp->next;
	}
	printf("\n");


}
