#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#define MAX_BUF 200

Players initNodeList(char* name, int sockDes) {
    Players L = (Players)malloc(sizeof(struct TList));
    L->name = (char*)malloc(MAX_BUF);
    strcpy(L->name,name);
    L->sockDes=sockDes;
    L->next = NULL;
    return L;
}

int isAlreadyLogged(Players L,char* name){
    int ret=0;
    if(L != NULL){
        if(strcmp(L->name,name)==0)
            return 1;
        ret=isAlreadyLogged(L->next,name);
    }
    return ret;
}

Players addPlayer(Players L, char* name,int sockDes) {
    Players tmp=initNodeList(name,sockDes);
    if (L != NULL) {
        tmp->next = L;     
    }
    return tmp;
}

Players removePlayer(Players L, int sockDes) {
    if (L != NULL) {
        if (L->sockDes==sockDes) {
            Players tmp = L->next;
            free(L);
            return tmp;
        }
        L->next = removePlayer(L->next, sockDes);
    }
    return L;
}


void freeList(Players L) {
    if (L != NULL) {
        freeList(L->next);
        free(L);
    }
}


void printList(Players L) {
    if (L != NULL) {
        printf("%s ->", L->name);
        printList(L->next);
    }
}