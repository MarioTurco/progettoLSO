#include "boardUtility.h"
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

void disconnettiClient();
int registraClient(int);
void timer(void *args);
void *gestisci(void *descriptor);
char grigliaDiGiocoConPacchiSenzaOstacoli[ROWS][COLUMNS];
char grigliaOstacoliSenzaPacchi[ROWS][COLUMNS];
int numeroClient = 0;
time_t timerCount = TIME_LIMIT_IN_SECONDS;
int main() {
  int socketDesc, clientDesc;
  int *thread_desc;
  pthread_t tid;
  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  mio_indirizzo.sin_addr.s_addr = htonl(INADDR_ANY);

  if ((socketDesc = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    perror("Impossibile creare socket"), exit(-1);
  if ((bind(socketDesc, (struct sockaddr *)&mio_indirizzo,
            sizeof(mio_indirizzo))) < 0)
    perror("Impossibile effettuare bind"), exit(-1);

  inizializzaGrigliaVuota(grigliaDiGiocoConPacchiSenzaOstacoli);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
      grigliaDiGiocoConPacchiSenzaOstacoli);
  generaPosizioneOstacoli(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi);
  while (1 == 1) {
    if (listen(socketDesc, 10) < 0)
      perror("Impossibile mettersi in ascolto"), exit(-1);
    printf("In ascolto..\n");
    if ((clientDesc = accept(socketDesc, NULL, NULL)) < 0) {
      perror("Impossibile effettuare connessione\n");
      exit(-1);
    }
    printf("%d", clientDesc);
    numeroClient++;
    printf("Connessione effettuata (totale client connessi: %d)\n",
           numeroClient);
    // creo un puntatore per il socket del client e lo passo al thread
    thread_desc = (int *)malloc(sizeof(int));
    *thread_desc = clientDesc;
    pthread_create(&tid, NULL, gestisci, (void *)thread_desc);
  }
  close(clientDesc);
  close(socketDesc);
  exit(0);
}

void *gestisci(void *descriptor) {
  int bufferSend[2] = {0};
  int bufferRecieve[2] = {1};
  int client_sd;
  int ret = 1;
  signal(SIGPIPE, disconnettiClient);
  client_sd = *(int *)descriptor;

  printf("server: gestisci sd = %d \n", client_sd);
  write(client_sd, bufferSend, 1);
  read(client_sd, bufferRecieve, sizeof(bufferRecieve));
  if (bufferRecieve[0] == 2) {
    if (registraClient(client_sd) < 0) {
      perror("Impossibile registrare utente, riprovare\n");
    }
    printf("Utente registrato con successo\n");
  } else if (bufferRecieve[0] == 1) {
    inserisciPlayerNellaGrigliaInPosizioneCasuale(
        grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi);
    write(client_sd, grigliaDiGiocoConPacchiSenzaOstacoli,
          sizeof(grigliaDiGiocoConPacchiSenzaOstacoli));
    /* while (1) {
       sleep(1);
       timer--;
       printf("%ld\n", timer);
       if (timer == 0) {
         inizializzaGrigliaVuota(grigliaDiGiocoConPacchiSenzaOstacoli);
         riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
             grigliaDiGiocoConPacchiSenzaOstacoli);
         generaPosizioneOstacoli(grigliaDiGiocoConPacchiSenzaOstacoli,
                                 grigliaOstacoliSenzaPacchi);
         inserisciPlayerNellaGrigliaInPosizioneCasuale(
             grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi);
         write(client_sd, grigliaDiGiocoConPacchiSenzaOstacoli,
               sizeof(grigliaDiGiocoConPacchiSenzaOstacoli));
         timer = TIME_LIMIT_IN_SECONDS;
       }
     }*/
    // userMovement();
  }
  close(client_sd);
  free(descriptor);
  printf("Client disconnesso (client attuali: %d)\n", numeroClient);
  pthread_exit(NULL);
}
void disconnettiClient() { numeroClient--; }
int registraClient(int clientDesc) {
  char userName[MAX_BUF];
  char password[MAX_BUF];
  int dimName,dimPwd;
  read(clientDesc,&dimName,sizeof(int));
  read(clientDesc,&dimPwd,sizeof(int));
  read(clientDesc, userName, dimName);
  read(clientDesc, password, dimPwd);
  printf("%s:%d\n%s:%d\n", userName,dimName,password,dimPwd);

  return 0;
}

void timer(void *args) {

  // TODO
}