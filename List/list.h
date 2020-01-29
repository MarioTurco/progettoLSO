#include <pthread.h>

struct TList {
    pthread_t tid;
    char* name;
    struct TList* next;
    int sockDes;
};

typedef struct TList* List;

// Inizializza un nuovo nodo
List initNodeList(pthread_t tid, char* name,int sockDes);

// Aggiunge un nodo in testa alla lista
// La funzione ritorna sempre la testa della lista
List addNodeHead(List L, pthread_t tid, char* name,int sockDes);

// Rimuove solo un occorrenza di un nodo con il tid specificato
// dalla lista
// La funzione ritorna sempre la testa della lista
List removeNodeList(List L, pthread_t tid);

// Dealloca la lista interamente
void freeList(List L);

// Stampa la lista
void printList(List L);

//Controlla se un utente è già loggato
int isAlreadyLogged(List L,char* name)
