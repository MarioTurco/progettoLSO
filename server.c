#include <netinet/in.h> //conversioni
#include <netinet/ip.h> //struttura
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void *threadFunction(void *string);
void *thr_fn(void *arg);
int main() {
  int numeroClient = 0;
  int socketDesc, clientDesc;
  pthread_t tid;
  struct sockaddr_in mio_indirizzo;
  char buffer[] = {"Saluti dal server\n"};
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  mio_indirizzo.sin_addr.s_addr = htonl(INADDR_ANY);

  socketDesc = socket(PF_INET, SOCK_STREAM, 0);
  bind(socketDesc, (struct sockaddr *)&mio_indirizzo, sizeof(mio_indirizzo));

  while (1) {
    listen(socketDesc, 2);
    if ((clientDesc = accept(socketDesc, NULL, NULL)) < 0) {
      perror("Impossibile effettuare connessione\n");
      exit(-1);
    }
    numeroClient++;
    printf("Connessione effettuata (totale client connessi: %d)\n",
           numeroClient);
    write(clientDesc, buffer, sizeof(buffer));
    pthread_create(&tid, NULL, threadFunction, "Prova\n");
  }
  close(clientDesc);
  close(socketDesc);
  exit(0);
}
void *thr_fn(void *arg) {
  printf("Nuovo thread\n");
  return ((void *)0);
}
void *threadFunction(void *string) {
  printf("Thread avviato\n");
  printf("%s", (char *)string);
  return (void *)0;
}