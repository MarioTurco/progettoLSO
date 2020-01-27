#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "list.h"
#define MAX_BUF 200

List initNodeList(pthread_t tid, char* name) {
    List L = (List)malloc(sizeof(struct TList));
    L->tid = tid;
    L->name = (char*)malloc(MAX_BUF);
    strcpy(L->name,name);
    L->next = NULL;
    return L;
}

List addNodeHead(List L, pthread_t tid, char* name) {
    List tmp=initNodeList(tid, name);
    if (L != NULL) {
        tmp->next = L;     
    }
    return tmp;
}

List removeNodeList(List L, pthread_t tid) {
    if (L != NULL) {
        if (pthread_equal(L->tid,tid)) {
            List tmp = L->next;
            free(L);
            return tmp;
        }
        L->next = removeNodeList(L->next, tid);
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