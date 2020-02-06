#include "boardUtility.h"
#include "list.h"
#include "parser.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h> //conversioni
#include <netinet/ip.h> //struttura
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
/*
TODO vanno incollate di nuovo tutte le firme perchè molto probabilmente molte sono cambiate e/o ne mancano un paio
*/
//retituisce 1 se c'è almeno un client connesso
int almenoUnClientConnesso();

//restituisce 1 se il timero è compreso tra 0 e TIME_LIMIT_IN_SECONDS
int valoreTimerValido();

//restituisce 1 se almeno 1 player è spawnato
int almenoUnPlayerGenerato();

//restituisce 1 se almeno una mossa è stata fatta da un player
int almenoUnaMossaFatta();

//invia il valore del timer(sotto forma di intero) a clientDesc
void sendTimerValue(int clientDesc);

//thread generazione nuovi player
void *threadGenerazioneNuoviPlayer(void *args);

//avvia il thread di generazione mappa
void startProceduraGenrazioneMappa();

//thread che genera la mappa
void *threadGenerazioneMappa(void *args);

//gestisce il login dell'utente
int tryLogin(int clientDesc, pthread_t tid);

//disconnette un client
void disconnettiClient(int);

//gestisce la registrazione del client
int registraClient(int);

//thread del timer
void *timer(void *args);

//thread di gestione dei client
void *gestisci(void *descriptor);

//chiude il server
void quitServer();

//handler del crash del client
void clientCrashHandler(int signalNum);

//fa partire il thread del timer
void startTimer();

//configura il socket
void configuraSocket(struct sockaddr_in mio_indirizzo);

//configura l'indirizzo ip
struct sockaddr_in configuraIndirizzo();

//ascolta le connessioni dei client in entrata
void startListening();

//controlla se un client si è disconnesso
int clientDisconnesso(int clientSocket);

//gestisce gli input da parte del client durante la partita
void play(int clientDesc, pthread_t tid);

//invia al client la lista dei player
void sendPlayerList(int clientDesc);

//gestisci la consegna del pacco del player
PlayerStats gestisciC(char grigliaDiGioco[ROWS][COLUMNS], PlayerStats giocatore,
                      Point deployCoords[], Point packsCoords[]);

//gestisci l'input del player
PlayerStats gestisciInput(char grigliaDiGioco[ROWS][COLUMNS],
                          char grigliaOstacoli[ROWS][COLUMNS], char input,
                          PlayerStats giocatore, Obstacles *listaOstacoli,
                          Point deployCoords[], Point packsCoords[]);
//clona una matrice
void clonaGriglia(char destinazione[ROWS][COLUMNS], char source[ROWS][COLUMNS]);
