
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
pthread_mutex_t RegMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PlayerMutex = PTHREAD_MUTEX_INITIALIZER;

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
void startProceduraGenrazioneMappa() {
  printf("Inizio procedura generazione mappa\n");
  pthread_create(&tidGeneratoreMappa, NULL, threadGenerazioneMappa, NULL);
}
void startTimer() {
  printf("Thread timer avviato\n");
  pthread_create(&tidTimer, NULL, timer, NULL);
}
int tryLogin(int clientDesc, char name[]) {
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
    pthread_mutex_lock(&PlayerMutex);
    onLineUsers = addPlayer(onLineUsers, userName, clientDesc);
    pthread_mutex_unlock(&PlayerMutex);
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
    } else {
      printf("Input invalido, uscita...\n");
      disconnettiClient(client_sd);
      break;
    }
  }
  pthread_exit(0);
}
void play(int clientDesc, char name[]) {
  int true = 1;
  int turnoFinito = 0;
  int turnoGiocatore = turno;
  int posizione[2];
  int destinazione[2] = {-1, -1};
  PlayerStats giocatore = initStats(destinazione, 0, posizione, 0);
  Obstacles listaOstacoli = NULL;
  char inputFromClient;
  if (timer != 0) {
    inserisciPlayerNellaGrigliaInPosizioneCasuale(
        grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
        giocatore->position);
    playerGenerati++;
  }
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
      freeObstacles(listaOstacoli);
      listaOstacoli = NULL;
      disconnettiClient(clientDesc);
    } else if (inputFromClient == 't' || inputFromClient == 'T') {
      write(clientDesc, &turnoFinito, sizeof(int));
      sendTimerValue(clientDesc);
    } else if (inputFromClient == 'l' || inputFromClient == 'L') {
      write(clientDesc, &turnoFinito, sizeof(int));
      sendPlayerList(clientDesc);
    } else if (turnoGiocatore == turno) {
      write(clientDesc, &turnoFinito, sizeof(int));
      giocatore =
          gestisciInput(grigliaDiGiocoConPacchiSenzaOstacoli,
                        grigliaOstacoliSenzaPacchi, inputFromClient, giocatore,
                        &listaOstacoli, deployCoords, packsCoords, name);
    } else {
      turnoFinito = 1;
      write(clientDesc, &turnoFinito, sizeof(int));
      freeObstacles(listaOstacoli);
      listaOstacoli = NULL;
      inserisciPlayerNellaGrigliaInPosizioneCasuale(
          grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
          giocatore->position);
      giocatore->score = 0;
      giocatore->hasApack = 0;
      giocatore->deploy[0] = -1;
      giocatore->deploy[1] = -1;
      turnoGiocatore = turno;
      turnoFinito = 0;
      playerGenerati++;
    }
  }
}
void sendTimerValue(int clientDesc) {
  if (!clientDisconnesso(clientDesc))
    write(clientDesc, &timerCount, sizeof(timerCount));
}
void clonaGriglia(char destinazione[ROWS][COLUMNS],
                  char source[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      destinazione[i][j] = source[i][j];
    }
  }
}
void clientCrashHandler(int signalNum) {
  char msg[0];
  int socketClientCrashato;
  int flag = 1;
  // TODO eliminare la lista degli ostacoli dell'utente
  if (onLineUsers != NULL) {
    Players prec = onLineUsers;
    Players top = prec->next;
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
  pthread_mutex_lock(&PlayerMutex);
  onLineUsers = removePlayer(onLineUsers, clientDescriptor);
  pthread_mutex_unlock(&PlayerMutex);
  printPlayers(onLineUsers);
  int msg = 1;
  printf("Client disconnesso (client attualmente loggati: %d)\n",
         numeroClientLoggati);
  write(clientDescriptor, &msg, sizeof(msg));
  close(clientDescriptor);
}
int clientDisconnesso(int clientSocket) {
  char msg[1] = {'u'}; // UP?
  if (write(clientSocket, msg, sizeof(msg)) < 0)
    return 1;
  if (read(clientSocket, msg, sizeof(char)) < 0)
    return 1;
  else
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
  pthread_mutex_lock(&RegMutex);
  int ret = appendPlayer(userName, password, users);
  pthread_mutex_unlock(&RegMutex);
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
  if (input == 'w' || input == 'W') {
    giocatore = gestisciW(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  } else if (input == 's' || input == 'S') {
    giocatore = gestisciS(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  } else if (input == 'a' || input == 'A') {
    giocatore = gestisciA(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  } else if (input == 'd' || input == 'D') {
    giocatore = gestisciD(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  } else if (input == 'p' || input == 'P') {
    giocatore = gestisciP(grigliaDiGioco, giocatore, deployCoords, packsCoords);
  } else if (input == 'c' || input == 'C') {
    giocatore =
        gestisciC(grigliaDiGioco, giocatore, deployCoords, packsCoords, name);
  }

  // aggiorna la posizione dell'utente
  return giocatore;
}

PlayerStats gestisciC(char grigliaDiGioco[ROWS][COLUMNS], PlayerStats giocatore,
                      Point deployCoords[], Point packsCoords[], char name[]) {
  pthread_t tid;
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
      if (giocatore->score > scoreMassimo)
        scoreMassimo = giocatore->score;
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
  int fDes = open("Log", O_RDWR | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);
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
