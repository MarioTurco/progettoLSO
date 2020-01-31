

struct TList { 
    char* name;
    struct TList* next;
    int sockDes;
};

struct TList2 { 
    int x;
    int y;
    struct TList2* next;
};

typedef struct TList* Players;
typedef struct TList2* Obstacles;

// Inizializza un nuovo nodo
Players initNodeList(char* name,int sockDes);

// Aggiunge un nodo in testa alla lista
// La funzione ritorna sempre la testa della lista
Players addPlayer(Players L, char* name,int sockDes);

// Rimuove solo un occorrenza di un nodo con il tid specificato
// dalla lista
// La funzione ritorna sempre la testa della lista
Players removePlayer(Players L, int sockDes);

// Dealloca la lista interamente
void freeList(Players L);

// Stampa la lista
void printList(Players L);

//Controlla se un utente è già loggato
int isAlreadyLogged(Players L,char* name);
