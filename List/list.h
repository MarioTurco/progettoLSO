#include <pthread.h>

struct TList {
    pthread_t tid;
    char* name;
    struct TList* next;
};

typedef struct TList* List;

// Inizializza un nuovo nodo
List initNodeList(pthread_t tid, char* name);

// Aggiunge un nodo in testa alla lista
// La funzione ritorna sempre la testa della lista
List addNodeHead(List L, pthread_t tid, char* name);

// Rimuove solo un occorrenza di un nodo con il tid specificato
// dalla lista
// La funzione ritorna sempre la testa della lista
List removeNodeList(List L, pthread_t tid);

// Dealloca la lista interamente
void freeList(List L);

// Stampa la lista
void printList(List L);
