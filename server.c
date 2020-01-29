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
#define MAX_BUF 100

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

/*//////////////////////////////////*/
char grigliaDiGiocoConPacchiSenzaOstacoli[ROWS][COLUMNS];
char grigliaOstacoliSenzaPacchi[ROWS][COLUMNS];
int numeroClient = 0;
time_t timerCount = TIME_LIMIT_IN_SECONDS;
pthread_t tidTimer;
int socketDesc;
List onLineUsers = NULL;
char *users;
/*///////////////////////////////*/

int main(int argc, char **argv) {
  int clientDesc;
  int *thread_desc;
  pthread_t tid;

  struct sockaddr_in mio_indirizzo = configuraIndirizzo();
  configuraSocket(mio_indirizzo);

  signal(SIGPIPE, clientCrashHandler);
  signal(SIGINT, quitServer);
  signal(SIGHUP, quitServer);

  if (argc != 2) {
    printf("Wrong parameters number(Usage: ./server usersFile)\n");
    exit(-1);
  }
  users = argv[1];

  inizializzaGiocoSenzaPlayer(grigliaDiGiocoConPacchiSenzaOstacoli,
                              grigliaOstacoliSenzaPacchi);

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
  return 0;
}
struct sockaddr_in configuraIndirizzo() {
  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  mio_indirizzo.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("Indirizzo socket configurato\n");
  return mio_indirizzo;
}
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
    printList(onLineUsers);
    printf("\n");
  }

  return ret;
}

void *gestisci(void *descriptor) {
  int bufferSend[2] = {0};
  int bufferReceive[2] = {1};
  int client_sd;
  int ret = 1;
  int posizione[2];
  client_sd = *(int *)descriptor;
  pthread_t tid = pthread_self();

  while (1) {
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
        int n;
        write(client_sd, "y", 1);

        inserisciPlayerNellaGrigliaInPosizioneCasuale(
            grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
            posizione);
        n = write(client_sd, grigliaDiGiocoConPacchiSenzaOstacoli,
                  sizeof(grigliaDiGiocoConPacchiSenzaOstacoli));
        printf("Size: %ld, sent %d\n", MATRIX_DIMENSION, n);
        int true = 1;
        while (true) {
          if (timerCount == TIME_LIMIT_IN_SECONDS) {
            inserisciPlayerNellaGrigliaInPosizioneCasuale(
                grigliaDiGiocoConPacchiSenzaOstacoli,
                grigliaOstacoliSenzaPacchi, posizione);
          }
          sleep(2); // al posto di questo sleep va messo un read
                    // così il server non invia continuamente ed il cliente non
                    // stampa continuamente il server deve aspettare l'input del
                    // client (tramite read che è bloccante) modificare la
                    // griglia in base all'input dell utente e reinviarla
                    // Poi lo faccio domani, ora non ho tempo per farlo
          write(client_sd, grigliaDiGiocoConPacchiSenzaOstacoli,
                sizeof(grigliaDiGiocoConPacchiSenzaOstacoli));
        }

        // userMovement();
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
  printf("uscita thread\n");
  pthread_exit(NULL);
}
void clientCrashHandler(int signalNum) {
  numeroClient--;
  printf("Client disconnesso (client attuali: %d)\n", numeroClient);
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
  printList(onLineUsers);
  printf("\n");
  int msg = 1;
  printf("Client disconnesso (client attuali: %d)\n", numeroClient);
  write(clientDescriptor, &msg, sizeof(msg));
  close(clientDescriptor);
  free(threadDescriptor);
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
void *timer(void *args) {
  int cambiato = 1;
  while (1) {
    if (numeroClient > 0 && timerCount > 0) {
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
      inizializzaGrigliaVuota(grigliaDiGiocoConPacchiSenzaOstacoli);
      riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
          grigliaDiGiocoConPacchiSenzaOstacoli);
      generaPosizioneOstacoli(grigliaDiGiocoConPacchiSenzaOstacoli,
                              grigliaOstacoliSenzaPacchi);
      timerCount = TIME_LIMIT_IN_SECONDS;
      sleep(2);
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