
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
#include <time.h>
#include <unistd.h>

struct argsToSend {
  char *userName;
  int flag;
};

typedef struct argsToSend *Args;

void sendPlayerList(int clientDesc);
PlayerStats gestisciC(char grigliaDiGioco[ROWS][COLUMNS], PlayerStats giocatore,
                      Point deployCoords[], Point packsCoords[], char name[]);
PlayerStats gestisciInput(char grigliaDiGioco[ROWS][COLUMNS],
                          char grigliaOstacoli[ROWS][COLUMNS], char input,
                          PlayerStats giocatore, Obstacles *listaOstacoli,
                          Point deployCoords[], Point packsCoords[],
                          char name[]);
void clonaGriglia(char destinazione[ROWS][COLUMNS], char source[ROWS][COLUMNS]);
int almenoUnClientConnesso();
int valoreTimerValido();
int almenoUnPlayerGenerato();
int almenoUnaMossaFatta();
void sendTimerValue(int clientDesc);
void *threadGenerazioneNuoviPlayer(void *args);
void startProceduraGenrazioneMappa();
void *threadGenerazioneMappa(void *args);
void *fileWriter(void *);
int tryLogin(int clientDesc, char name[]);
void disconnettiClient(int);
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
void play(int clientDesc, char name[]);
/*//////////////////////////////////*/
char grigliaDiGiocoConPacchiSenzaOstacoli[ROWS][COLUMNS];
char grigliaOstacoliSenzaPacchi[ROWS][COLUMNS];
int numeroClientLoggati = 0;
int playerGenerati = 0;
int timerCount = TIME_LIMIT_IN_SECONDS;
int turno = 0;
pthread_t tidTimer;
pthread_t tidGeneratoreMappa;
int socketDesc;
Players onLineUsers = NULL;
char *users;
int scoreMassimo = 0;
int numMosse = 0;
Point deployCoords[numberOfPackages];
Point packsCoords[numberOfPackages];
pthread_mutex_t LogMutex = PTHREAD_MUTEX_INITIALIZER;
/*///////////////////////////////*/

int main(int argc, char **argv) {

  if (argc != 2) {
    printf("Wrong parameters number(Usage: ./server usersFile)\n");
    exit(-1);
  } else if (strcmp(argv[1], "Log") == 0) {
    printf("Cannot use the Log file as a UserList \n");
    exit(-1);
  }

  users = argv[1];
  struct sockaddr_in mio_indirizzo = configuraIndirizzo();
  configuraSocket(mio_indirizzo);

  signal(SIGPIPE, clientCrashHandler);
  signal(SIGINT, quitServer);
  signal(SIGHUP, quitServer);

  startTimer();
  inizializzaGiocoSenzaPlayer(grigliaDiGiocoConPacchiSenzaOstacoli,
                              grigliaOstacoliSenzaPacchi, packsCoords);
  generaPosizioniRaccolta(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi, deployCoords);
  startListening();
  return 0;
}

void startListening() {
  pthread_t tid;
  int clientDesc;
  int *puntClientDesc;
  while (1 == 1) {
    if (listen(socketDesc, 10) < 0)
      perror("Impossibile mettersi in ascolto"), exit(-1);
    printf("In ascolto..\n");
    if ((clientDesc = accept(socketDesc, NULL, NULL)) < 0) {
      perror("Impossibile effettuare connessione\n");
      exit(-1);
    }
    printf("Nuovo client connesso\n");
    // creo un puntatore per il socket del client e lo passo al thread
    puntClientDesc = (int *)malloc(sizeof(int));
    *puntClientDesc = clientDesc;
    pthread_create(&tid, NULL, gestisci, (void *)puntClientDesc);
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
int tryLogin(int clientDesc, char name[]) {
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
    numeroClientLoggati++;
    write(clientDesc, "y", 1);
    strcpy(name, userName);
    Args args = (Args)malloc(sizeof(struct argsToSend));
    args->userName = (char *)calloc(MAX_BUF, 1);
    strcpy(args->userName, name);
    args->flag = 0;
    pthread_t tid;
    pthread_create(&tid, NULL, fileWriter, (void *)args);
    printf("Nuovo client loggato, client loggati : %d\n", numeroClientLoggati);
    // TODO: proteggere con un mutex
    onLineUsers = addPlayer(onLineUsers, userName, clientDesc);
    printPlayers(onLineUsers);
    printf("\n");
  } else {
    write(clientDesc, "n", 1);
  }
  return ret;
}

void *gestisci(void *descriptor) {
  int bufferReceive[2] = {1};
  int client_sd = *(int *)descriptor;
  int continua = 1;
  char name[MAX_BUF];
  while (continua) {
    read(client_sd, bufferReceive, sizeof(bufferReceive));
    if (bufferReceive[0] == 2) {
      registraClient(client_sd);
    }

    else if (bufferReceive[0] == 1) {
      if (tryLogin(client_sd, name)) {
        play(client_sd, name);
        continua = 0;
      }
    } else if (bufferReceive[0] == 3) {
      disconnettiClient(client_sd);
      break;
    }
    // TODO Aggiungere un opzione per stampare tutte le impostazioni
    else {
      printf("Input invalido, uscita...\n");
      disconnettiClient(client_sd);
      break;
    }
  }
  pthread_exit(0);
}

void play(int clientDesc, char name[]) {
  int true = 1;
  int turnoGiocatore = turno;
  int posizione[2];
  int destinazione[2] = {-1, -1};
  PlayerStats giocatore = initStats(destinazione, 0, posizione, 0);
  Obstacles listaOstacoli = NULL;
  // pthread_t tidGenerazionePlayer;
  char inputFromClient;
  int punteggio = 0;
  if (timer != 0) {
    inserisciPlayerNellaGrigliaInPosizioneCasuale(
        grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
        giocatore->position);
    playerGenerati++;
  }
  // pthread_create(&tidGenerazionePlayer, NULL, threadGenerazioneNuoviPlayer,
  //                (void *)giocatore);
  while (true) {
    if (clientDisconnesso(clientDesc)) {
      freeObstacles(listaOstacoli);
      disconnettiClient(clientDesc);
      return;
    }
    char grigliaTmp[ROWS][COLUMNS];
    clonaGriglia(grigliaTmp, grigliaDiGiocoConPacchiSenzaOstacoli);
    mergeGridAndList(grigliaTmp, listaOstacoli);
    // invia la griglia
    write(clientDesc, grigliaTmp, sizeof(grigliaTmp));
    // invia la struttura del player
    write(clientDesc, giocatore->deploy, sizeof(giocatore->deploy));
    write(clientDesc, giocatore->position, sizeof(giocatore->position));
    write(clientDesc, &giocatore->score, sizeof(giocatore->score));
    write(clientDesc, &giocatore->hasApack, sizeof(giocatore->hasApack));
    sendTimerValue(clientDesc);
    // legge l'input
    if (read(clientDesc, &inputFromClient, sizeof(char)) > 0)
      numMosse++;
    if (inputFromClient == 'e' || inputFromClient == 'E') {
      // TODO svuotare la lista obstacles quando si disconnette un client
      freeObstacles(listaOstacoli);
      listaOstacoli = NULL;
      disconnettiClient(clientDesc);
    } else if (inputFromClient == 't' || inputFromClient == 'T') {
      sendTimerValue(clientDesc);
    } else if (inputFromClient == 'l' || inputFromClient == 'L') {
      sendPlayerList(clientDesc);
    } else if (turnoGiocatore == turno)
      giocatore =
          gestisciInput(grigliaDiGiocoConPacchiSenzaOstacoli,
                        grigliaOstacoliSenzaPacchi, inputFromClient, giocatore,
                        &listaOstacoli, deployCoords, packsCoords, name);
    else {
      printObstacles(listaOstacoli);
      freeObstacles(listaOstacoli);
      listaOstacoli = NULL;
      printObstacles(listaOstacoli);
      printObstacles(listaOstacoli);
      inserisciPlayerNellaGrigliaInPosizioneCasuale(
          grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
          giocatore->position);
      giocatore->score = 0;
      giocatore->hasApack = 0;
      giocatore->deploy[0] = -1;
      giocatore->deploy[1] = -1;
      turnoGiocatore = turno;
      playerGenerati++;
    }
  }
}
void sendTimerValue(int clientDesc) {
  if (!clientDisconnesso(clientDesc)) {
    write(clientDesc, &timerCount, sizeof(timerCount));
  }
}
void clonaGriglia(char destinazione[ROWS][COLUMNS],
                  char source[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      destinazione[i][j] = source[i][j];
    }
  }

  return;
}
// TODO da cancellare, non serve più
/*void *threadGenerazioneNuoviPlayer(void *args) {
  timerCount = TIME_LIMIT_IN_SECONDS;
  PlayerStats giocatore = (PlayerStats)args;
  while (1) {
    if (timerCount == TIME_LIMIT_IN_SECONDS + 1) {
      inserisciPlayerNellaGrigliaInPosizioneCasuale(
          grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
          giocatore->position);
      giocatore->score = 0;
      giocatore->hasApack = 0;
      giocatore->deploy[0] = -1;
      giocatore->deploy[1] = -1;
      playerGenerati++;
      if (playerGenerati == numeroClientLoggati) {
        timerCount = TIME_LIMIT_IN_SECONDS;
        playerGenerati = 0;
      }
    }
  }
}*/
void clientCrashHandler(int signalNum) {
  char msg[0];
  int socketClientCrashato;
  int flag = 1;
  // TODO eliminare la lista degli ostacoli dell'utente
  // elimina il client dalla lista dei client connessi
  if (onLineUsers != NULL) {
    Players prec = onLineUsers;
    Players top = prec->next;
    // controlla se è crashato il top
    while (top != NULL && flag) {
      if (write(top->sockDes, msg, sizeof(msg)) < 0) {
        socketClientCrashato = top->sockDes;
        printPlayers(onLineUsers);
        disconnettiClient(socketClientCrashato);
        flag = 0;
      }
      top = top->next;
    }
  }
  signal(SIGPIPE, SIG_IGN);
}
void disconnettiClient(int clientDescriptor) {
  if (numeroClientLoggati > 0)
    numeroClientLoggati--;
  // TODO proteggere con un mutex
  onLineUsers = removePlayer(onLineUsers, clientDescriptor);
  printPlayers(onLineUsers);
  int msg = 1;
  printf("Client disconnesso (client attualmente loggati: %d)\n",
         numeroClientLoggati);
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
  char risposta;
  if (!ret) {
    risposta = 'n';
    write(clientDesc, &risposta, sizeof(char));
    printf("Impossibile registrare utente, riprovare\n");
  } else {

    risposta = 'y';
    write(clientDesc, &risposta, sizeof(char));
    printf("Utente registrato con successo\n");
  }
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
  generaPosizioniRaccolta(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi, deployCoords);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
      grigliaDiGiocoConPacchiSenzaOstacoli, packsCoords);
  generaPosizioneOstacoli(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi);
  timerCount = TIME_LIMIT_IN_SECONDS;
  printf("Mappa generata\n");
  pthread_exit(NULL);
}
int almenoUnaMossaFatta() {
  if (numMosse > 0)
    return 1;
  return 0;
}
int almenoUnClientConnesso() {
  if (numeroClientLoggati > 0)
    return 1;
  return 0;
}
int valoreTimerValido() {
  if (timerCount > 0 && timerCount <= TIME_LIMIT_IN_SECONDS)
    return 1;
  return 0;
}
int almenoUnPlayerGenerato() {
  if (playerGenerati > 0)
    return 1;
  return 0;
}
void *timer(void *args) {
  int cambiato = 1;
  while (1) {
    if (almenoUnClientConnesso() && valoreTimerValido() &&
        almenoUnPlayerGenerato() && almenoUnaMossaFatta()) {
      cambiato = 1;
      sleep(1);
      timerCount--;
      fprintf(stdout, "Time left: %d\n", timerCount);
    } else if (numeroClientLoggati == 0) {
      timerCount = TIME_LIMIT_IN_SECONDS;
      if (cambiato) {
        fprintf(stdout, "Time left: %d\n", timerCount);
        cambiato = 0;
      }
    }
    if (timerCount == 0 || scoreMassimo == packageLimitNumber) {
      playerGenerati = 0;
      numMosse = 0;
      printf("Reset timer e generazione nuova mappa..\n");
      startProceduraGenrazioneMappa();
      pthread_join(tidGeneratoreMappa, NULL);
      turno++;
      timerCount = TIME_LIMIT_IN_SECONDS;
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

PlayerStats gestisciInput(char grigliaDiGioco[ROWS][COLUMNS],
                          char grigliaOstacoli[ROWS][COLUMNS], char input,
                          PlayerStats giocatore, Obstacles *listaOstacoli,
                          Point deployCoords[], Point packsCoords[],
                          char name[]) {
  if (giocatore == NULL) {
    return NULL;
  }
  PlayerStats nuoveStatistiche =
      initStats(giocatore->deploy, giocatore->score, giocatore->position,
                giocatore->hasApack);
  if (input == 'w' || input == 'W') {
    nuoveStatistiche = gestisciW(grigliaDiGioco, grigliaOstacoli, giocatore,
                                 listaOstacoli, deployCoords, packsCoords);
  } else if (input == 's' || input == 'S') {
    nuoveStatistiche = gestisciS(grigliaDiGioco, grigliaOstacoli, giocatore,
                                 listaOstacoli, deployCoords, packsCoords);
  } else if (input == 'a' || input == 'A') {
    nuoveStatistiche = gestisciA(grigliaDiGioco, grigliaOstacoli, giocatore,
                                 listaOstacoli, deployCoords, packsCoords);
  } else if (input == 'd' || input == 'D') {
    nuoveStatistiche = gestisciD(grigliaDiGioco, grigliaOstacoli, giocatore,
                                 listaOstacoli, deployCoords, packsCoords);
  } else if (input == 'p' || input == 'P') {
    nuoveStatistiche =
        gestisciP(grigliaDiGioco, giocatore, deployCoords, packsCoords);
  } else if (input == 'c' || input == 'C') {
    nuoveStatistiche =
        gestisciC(grigliaDiGioco, giocatore, deployCoords, packsCoords, name);
  }

  // aggiorna la posizione dell'utente
  return nuoveStatistiche;
}

PlayerStats gestisciC(char grigliaDiGioco[ROWS][COLUMNS], PlayerStats giocatore,
                      Point deployCoords[], Point packsCoords[], char name[]) {
  pthread_t tid;
  // il secondo NULL è il parametro da passare alla funzione NULL = nessun
  // parametro
  if (giocatore->hasApack == 0) {
    return giocatore;
  } else {
    if (isOnCorrectDeployPoint(giocatore, deployCoords)) {
      Args args = (Args)malloc(sizeof(struct argsToSend));
      args->userName = (char *)calloc(MAX_BUF, 1);
      strcpy(args->userName, name);
      args->flag = 1;
      pthread_create(&tid, NULL, fileWriter, (void *)args);
      giocatore->score += 10;
      giocatore->deploy[0] = -1;
      giocatore->deploy[1] = -1;
      giocatore->hasApack = 0;
    } else {
      if (!isOnAPack(giocatore, packsCoords) &&
          !isOnADeployPoint(giocatore, deployCoords)) {
        int index = getHiddenPack(packsCoords);
        if (index >= 0) {
          packsCoords[index]->x = giocatore->position[0];
          packsCoords[index]->y = giocatore->position[1];
          giocatore->hasApack = 0;
        }
      } else
        return giocatore;
    }
  }
  return giocatore;
}

void sendPlayerList(int clientDesc) {
  int lunghezza = 0;
  char name[100];
  Players tmp = onLineUsers;
  int numeroClientLoggati = dimensioneLista(tmp);
  printf("%d ", numeroClientLoggati);
  if (!clientDisconnesso(clientDesc)) {
    write(clientDesc, &numeroClientLoggati, sizeof(numeroClientLoggati));
    while (numeroClientLoggati > 0 && tmp != NULL) {
      strcpy(name, tmp->name);
      lunghezza = strlen(tmp->name);
      write(clientDesc, &lunghezza, sizeof(lunghezza));
      write(clientDesc, name, lunghezza);
      tmp = tmp->next;
      numeroClientLoggati--;
    }
  }
}

void *fileWriter(void *args) {
  int fDes = open("Log", O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);
  if (fDes < 0) {
    perror("Error while opening log file");
    exit(-1);
  }
  Args info = (Args)args;
  time_t t = time(NULL);
  struct tm *infoTime = localtime(&t);
  char toPrint[64];
  strftime(toPrint, sizeof(toPrint), "%X %x", infoTime);
  if (info->flag == 1) {
    char message[MAX_BUF] = "Pack delivered by \"";
    strcat(message, info->userName);
    char at[] = "\" at ";
    strcat(message, at);
    strcat(message, toPrint);
    strcat(message, "\n");
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  } else if (info->flag == 0) {
    char message[MAX_BUF] = "\"";
    strcat(message, info->userName);
    strcat(message, "\" connected at ");
    strcat(message, toPrint);
    strcat(message, "\n");
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  }
  close(fDes);
  pthread_exit(NULL);
}
