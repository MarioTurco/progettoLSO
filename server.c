#include <netinet/in.h> //conversioni
#include <netinet/ip.h> //struttura
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  int numeroClient = 0;
  int socketDesc, clientDesc;
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
  }
  close(clientDesc);
  close(socketDesc);
  exit(0);
}