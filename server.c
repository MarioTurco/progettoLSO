#include "boardUtility.h"
#include "list.h"
#include "parser.h"
#include <netinet/in.h> //conversioni
#include <netinet/ip.h> //struttura
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void startProceduraGenrazioneMappa();
void *threadGenerazioneMappa(void *args);
int tryLogin(int clientDescriptor);
void disconnettiClient();
int registraClient(int);
void *timer(void *args);
void *gestisci(void *descriptor);
void quitServer();
void clientCrashHandler(int signalNum);
void startTimer();
void configuraSocket(struct sockaddr_in mio_indirizzo);
struct sockaddr_in configuraIndirizzo();
void startListening();
int clientDisconnesso(int clientSocket);
void play(int clientDesc, pthread_t tid);
/*//////////////////////////////////*/
char grigliaDiGiocoConPacchiSenzaOstacoli[ROWS][COLUMNS];
char grigliaOstacoliSenzaPacchi[ROWS][COLUMNS];
int numeroClient = 0;
int playerGenerati = 0;
time_t timerCount = TIME_LIMIT_IN_SECONDS;
pthread_t tidTimer;
pthread_t tidGeneratoreMappa;
int socketDesc;
Players onLineUsers = NULL;
char *users;
/*///////////////////////////////*/

int main(int argc, char **argv) {

  users = argv[1];
  struct sockaddr_in mio_indirizzo = configuraIndirizzo();
  configuraSocket(mio_indirizzo);

  signal(SIGPIPE, clientCrashHandler);
  signal(SIGINT, quitServer);
  signal(SIGHUP, quitServer);

  if (argc != 2) {
    printf("Wrong parameters number(Usage: ./server usersFile)\n");
    exit(-1);
  }
  startTimer();
  inizializzaGiocoSenzaPlayer(grigliaDiGiocoConPacchiSenzaOstacoli,
                              grigliaOstacoliSenzaPacchi);
  startListening();
  return 0;
}
void startListening() {
  pthread_t tid;
  int clientDesc;
  int *thread_desc;
  while (1 == 1) {
    if (listen(socketDesc, 10) < 0)
      perror("Impossibile mettersi in ascolto"), exit(-1);
    printf("In ascolto..\n");
    if ((clientDesc = accept(socketDesc, NULL, NULL)) < 0) {
      perror("Impossibile effettuare connessione\n");
      exit(-1);
    }
    printf("Connessione effettuata (totale client connessi: %d)\n",
           numeroClient);
    // creo un puntatore per il socket del client e lo passo al thread
    thread_desc = (int *)malloc(sizeof(int));
    *thread_desc = clientDesc;
    pthread_create(&tid, NULL, gestisci, (void *)thread_desc);
  }
  close(clientDesc);
  quitServer();
}

struct sockaddr_in configuraIndirizzo() {
  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  mio_indirizzo.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("Indirizzo socket configurato\n");
  return mio_indirizzo;
}

/* Genera una nuova mappa appena il timer arriva a 0*/
void startProceduraGenrazioneMappa() {
  printf("Inizio procedura generazione mappa\n");
  pthread_create(&tidGeneratoreMappa, NULL, threadGenerazioneMappa, NULL);
}
/* Inizia un count down*/
void startTimer() {
  printf("Thread timer avviato\n");
  pthread_create(&tidTimer, NULL, timer, NULL);
}
int tryLogin(int clientDesc) {
  // TODO proteggere con un mutex
  char *userName = (char *)calloc(MAX_BUF, 1);
  char *password = (char *)calloc(MAX_BUF, 1);
  int dimName, dimPwd;
  read(clientDesc, &dimName, sizeof(int));
  read(clientDesc, &dimPwd, sizeof(int));
  read(clientDesc, userName, dimName);
  read(clientDesc, password, dimPwd);

  int ret = 0;
  if (validateLogin(userName, password, users) &&
      !isAlreadyLogged(onLineUsers, userName)) {
    ret = 1;
    numeroClient++;
    printf("Nuovo client loggato, client loggati : %d\n", numeroClient);
    // TODO: proteggere con un mutex
    onLineUsers = addPlayer(onLineUsers, userName, clientDesc);
    printPlayers(onLineUsers);
    printf("\n");
  }
  return ret;
}

void *gestisci(void *descriptor) {
  int bufferSend[2] = {0};
  int bufferReceive[2] = {1};
  int client_sd;
  int ret = 1;
  int true = 1;
  client_sd = *(int *)descriptor;
  pthread_t tid = pthread_self();

  while (true) {
    read(client_sd, bufferReceive, sizeof(bufferReceive));
    if (bufferReceive[0] == 2) {
      int ret = registraClient(client_sd);
      char risposta;
      if (!ret) {
        risposta = 'n';
        write(client_sd, &risposta, sizeof(char));
        printf("Impossibile registrare utente, riprovare\n");
      } else {

        risposta = 'y';
        write(client_sd, &risposta, sizeof(char));
        printf("Utente registrato con successo\n");
      }
    }

    else if (bufferReceive[0] == 1) {
      int grantAccess = tryLogin(client_sd);

      if (grantAccess) {
        write(client_sd, "y", 1);
        play(client_sd, tid);
        true = 0;
      } else {
        write(client_sd, "n", 1);
      }
    }

    else if (bufferReceive[0] == 3) {
      disconnettiClient(client_sd, descriptor);
      break;
    }

    else {
      printf("Input invalido, uscita...\n");
      disconnettiClient(client_sd, descriptor);
      break;
    }
  }
}

void play(int clientDesc, pthread_t tid) {
  int true = 1;
  int posizione[2];
  int destinazione[2] = {-1, -1};
  PlayerStats giocatore = initStats(destinazione, 0, posizione);
  Obstacles listaOstacoli = NULL;
  char inputFromClient;
  int punteggio = 0;
  if (timer != 0) {
    inserisciPlayerNellaGrigliaInPosizioneCasuale(
        grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
        giocatore->position);
  }
  while (true) {
    if (clientDisconnesso(clientDesc)) {
      disconnettiClient(clientDesc, tid);
      return;
    }
    if (timerCount == TIME_LIMIT_IN_SECONDS + 1) {
      inserisciPlayerNellaGrigliaInPosizioneCasuale(
          grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
          giocatore->position);
      playerGenerati++;
      if (playerGenerati == numeroClient) {
        timerCount = TIME_LIMIT_IN_SECONDS;
        playerGenerati = 0;
      }
    }
    //printObstacles(listaOstacoli);
    mergeGridAndList(grigliaDiGiocoConPacchiSenzaOstacoli,listaOstacoli);
    // invia la griglia
    write(clientDesc, grigliaDiGiocoConPacchiSenzaOstacoli,
          sizeof(grigliaDiGiocoConPacchiSenzaOstacoli));
    // invia la struttura del player
    write(clientDesc, giocatore->deploy, sizeof(giocatore->deploy));
    write(clientDesc, giocatore->position, sizeof(giocatore->position));
    write(clientDesc, &giocatore->score, sizeof(giocatore->score));

    // legge l'input
    read(clientDesc, &inputFromClient, sizeof(char));
    printf("Inserito: %c", inputFromClient);
    if (inputFromClient == 'e' || inputFromClient == 'E') {
      //TODO svuotare la lista obstacles quando si disconnette un client
      disconnettiClient(clientDesc, tid);
    } else
      giocatore = gestisciInput(grigliaDiGiocoConPacchiSenzaOstacoli,
                                grigliaOstacoliSenzaPacchi, inputFromClient,
                                giocatore, &listaOstacoli);
  }
}
void clientCrashHandler(int signalNum) {
  if (numeroClient > 0) {
    numeroClient--;
    printf("Client disconnesso (client attuali: %d)\n", numeroClient);
  }
  pthread_t tidDelServerCrashato;
  char msg[1] = {'U'};
  int socketClientCrashato;
  if(onLineUsers != NULL){
    Players prec = onLineUsers;
    Players top = prec->next;
    //controlla se è crashato il top
     while(top!=NULL){
      if(write(top->sockDes, msg, sizeof(msg))<0){
        socketClientCrashato = top->sockDes;
        close(socketClientCrashato);
        onLineUsers = removePlayer(onLineUsers, socketClientCrashato);
        break;
      }
    } 
  }
  // TODO proteggere con un mutex
  // onLineUsers = removePlayer(onLineUsers, clientDescriptor); //trovare il
  // modo per cancellare il player giusto printList(onLineUsers); printf("\n");
  signal(SIGPIPE, SIG_IGN);
}
void disconnettiClient(int clientDescriptor, int *threadDescriptor) {
  if (numeroClient > 0)
    numeroClient--;
  // TODO proteggere con un mutex
  onLineUsers = removePlayer(onLineUsers, clientDescriptor);
  printPlayers(onLineUsers);
  printf("\n");
  int msg = 1;
  printf("Client disconnesso (client attuali: %d)\n", numeroClient);
  write(clientDescriptor, &msg, sizeof(msg));
  close(clientDescriptor);
}

int clientDisconnesso(int clientSocket) {
  char msg[1] = {'u'}; // UP?
  if (write(clientSocket, msg, sizeof(msg)) < 0) {
    return 1;
  }
  if (read(clientSocket, msg, sizeof(char)) < 0) {
    return 1;
  } else
    return 0;
}
int registraClient(int clientDesc) {
  char *userName = (char *)calloc(MAX_BUF, 1);
  char *password = (char *)calloc(MAX_BUF, 1);
  int dimName, dimPwd;
  read(clientDesc, &dimName, sizeof(int));
  read(clientDesc, &dimPwd, sizeof(int));
  read(clientDesc, userName, dimName);
  read(clientDesc, password, dimPwd);
  // printf("%s:%d\n%s:%d\n", userName, dimName, password, dimPwd);
  // TODO proteggere con un mutex
  int ret = appendPlayer(userName, password, users);
  return ret;
}

void quitServer() {
  printf("Chiusura server in corso..\n");
  close(socketDesc);
  exit(-1);
}

void *threadGenerazioneMappa(void *args) {
  fprintf(stdout, "Rigenerazione mappa\n");
  inizializzaGrigliaVuota(grigliaDiGiocoConPacchiSenzaOstacoli);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
      grigliaDiGiocoConPacchiSenzaOstacoli);
  generaPosizioneOstacoli(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi);
  timerCount = TIME_LIMIT_IN_SECONDS;
  pthread_exit(NULL);
}
void *timer(void *args) {
  int cambiato = 1;
  while (1) {
    if (numeroClient > 0 && timerCount > 0 &&
        timerCount <= TIME_LIMIT_IN_SECONDS) {
      cambiato = 1;
      sleep(1);
      timerCount--;
      fprintf(stdout, "Time left: %ld\n", timerCount);
    } else if (numeroClient == 0) {
      timerCount = TIME_LIMIT_IN_SECONDS;
      if (cambiato) {
        fprintf(stdout, "Time left: %ld\n", timerCount);
        cambiato = 0;
      }
    }
    if (timerCount == 0) {
      printf("Reset timer e generazione nuova mappa..\n");
      startProceduraGenrazioneMappa();
      pthread_join(tidGeneratoreMappa, NULL);
      timerCount = TIME_LIMIT_IN_SECONDS + 1;
    }
  }
}

void configuraSocket(struct sockaddr_in mio_indirizzo) {
  if ((socketDesc = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Impossibile creare socket");
    exit(-1);
  }
  if (setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <
      0)
    perror("Impossibile impostare il riutilizzo dell'indirizzo ip e della "
           "porta\n");
  if ((bind(socketDesc, (struct sockaddr *)&mio_indirizzo,
            sizeof(mio_indirizzo))) < 0) {
    perror("Impossibile effettuare bind");
    exit(-1);
  }
}
