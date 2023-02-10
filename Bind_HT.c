#include <math.h>
#include "Names_List.c"
#define _CRT_SECURE_NO_WARNINGS
#define SIZE 100

enum{DEC_ERROR,TYPE_MISS, UNDECLERED_ID,COND_MISSTYPE,INDEX_MISSTYPE,STRLEN_MISSTYPE,STR_INDEX,FUNCDEC_ERROR,UNINITIAL_ID,RETURN_TYPE,MAINFUNC_ERROR};
//	Hash Table struct definition
typedef struct Bind_HT
{
	Names_List** hashTable;
}Bind_HT;


Bind_HT* initTable()
{//	Init the hash table
	Bind_HT* ht = (Bind_HT*)malloc(sizeof(Bind_HT));	//	Allocate memory
    ht->hashTable=(Names_List**)malloc(SIZE * sizeof(Names_List*));	//	Allocate memory
	return ht;
}

int hash(char* n){
    int sum=0;
    for(int i=0;i<strlen(n);i++){
        sum+=n[i]-'0';
    }
    return sum % SIZE;

}

Bind_HT* addName(Bind_HT* ht,char* n, int scope){

    int index= hash(n);
    if(ht->hashTable[index]==NULL)
        ht->hashTable[index]=BuildNode(n,scope);
    else{
        if(isNameExists(ht->hashTable[index],n)!=-1)
            ht->hashTable[index]=AddScopeToName(ht->hashTable[index],n,scope);
        else
            ht->hashTable[index]=addToStart(ht->hashTable[index],n,scope);   
    }
    return ht; 
    
}




int getScope(Bind_HT* ht,char* n){
    int index = hash(n), scope = -1;
    if(ht->hashTable[index] != NULL)
        {
            scope = isNameExists(ht->hashTable[index], n);
        }
    return scope;
}

Bind_HT*  deleteScopeFromHT(Bind_HT* ht,int scope){
    for(int i=0;i<SIZE;i++){
        if( ht->hashTable[i])
            ht->hashTable[i]=deleteScopeNames(ht->hashTable[i],scope);
    }
    return ht;
}

void FreeHashTable(Bind_HT * ht)
{

    for(int i=0;i<SIZE;i++)
        if(ht->hashTable[i]!=NULL)
            FreeList(ht->hashTable[i]);
	free(ht->hashTable);
    free(ht);

}

void printBindHT(Bind_HT* ht){
    printf("BIND_HT:\n");
    for(int i=0;i<SIZE;i++){
        if(ht->hashTable[i]!=NULL){
            printf("Bind_HT[%d] :\n",i);
            printNamesList(ht->hashTable[i]);
        }
    }
        
}

Bind_HT* addScope(Bind_HT* ht, char* n, int scope){
    int index= hash(n);
    ht->hashTable[index]=AddScopeToName(ht->hashTable[index],n,scope);
    return ht;
}

int isNameInBindHT(Bind_HT* ht, char* name,int new_bind_scope){
    int index=hash(name);
    int scope=isNameExists(ht->hashTable[index],name);
    if(scope==new_bind_scope) 
        return 1;
    return 0;
}