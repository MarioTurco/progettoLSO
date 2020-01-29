

struct TList { 
    char* name;
    struct TList* next;
    int sockDes;
};

typedef struct TList* List;

// Inizializza un nuovo nodo
List initNodeList(char* name,int sockDes);

// Aggiunge un nodo in testa alla lista
// La funzione ritorna sempre la testa della lista
List addPlayer(List L, char* name,int sockDes);

// Rimuove solo un occorrenza di un nodo con il tid specificato
// dalla lista
// La funzione ritorna sempre la testa della lista
List removePlayer(List L, int sockDes);

// Dealloca la lista interamente
void freeList(List L);

// Stampa la lista
void printList(List L);

//Controlla se un utente è già loggato
int isAlreadyLogged(List L,char* name);
