// 93.46.102.184
#include <arpa/inet.h>
#include <netinet/in.h> //conversioni
#include <netinet/ip.h> //struttura
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define ipaddr "localhost"
int main() {
  int socketDesc;
  char buffer[25];
  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  inet_aton(ipaddr, &mio_indirizzo.sin_addr);

  socketDesc = socket(PF_INET, SOCK_STREAM, 0);
  if (connect(socketDesc, (struct sockaddr *)&mio_indirizzo,
              sizeof(mio_indirizzo)) < 0)
    perror("Impossibile connettersi"), exit(-1);
  if (read(socketDesc, buffer, 20) < 0) {
    printf("impossibile leggere il messaggio\n");
  }
  printf("%s\n", buffer);
  close(socketDesc);
  exit(0);
}