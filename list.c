#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#define MAX_BUF 200

List initNodeList(char* name, int sockDes) {
    List L = (List)malloc(sizeof(struct TList));
    L->name = (char*)malloc(MAX_BUF);
    strcpy(L->name,name);
    L->sockDes=sockDes;
    L->next = NULL;
    return L;
}

int isAlreadyLogged(List L,char* name){
    int ret=0;
    if(L != NULL){
        if(strcmp(L->name,name)==0)
            ret=1;
        else  ret=0;
        ret=isAlreadyLogged(L->next,name);
    }
    return ret;
}

List addPlayer(List L, char* name,int sockDes) {
    List tmp=initNodeList(name,sockDes);
    if (L != NULL) {
        tmp->next = L;     
    }
    return tmp;
}

List removePlayer(List L, int sockDes) {
    if (L != NULL) {
        if (L->sockDes==sockDes) {
            List tmp = L->next;
            free(L);
            return tmp;
        }
        L->next = removePlayer(L->next, sockDes);
    }
    return L;
}


void freeList(List L) {
    if (L != NULL) {
        freeList(L->next);
        free(L);
    }
}


void printList(List L) {
    if (L != NULL) {
        printf("%s ", L->name);
        printList(L->next);
    }
}