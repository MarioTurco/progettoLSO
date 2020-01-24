#include "scacchiera.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h> //conversioni
#include <netinet/ip.h> //struttura
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ipaddr "127.0.0.1"
char *retriveIpAddressFromFile();
int registrati(int);
int gestisci(int, int);
char getUserInput();
int login();
int main() {
  int socketDesc;
  char bufferRecieve[2];

  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  inet_aton(ipaddr, &mio_indirizzo.sin_addr);
  if ((socketDesc = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    perror("Impossibile creare socket"), exit(-1);
  if (connect(socketDesc, (struct sockaddr *)&mio_indirizzo,
              sizeof(mio_indirizzo)) < 0)
    perror("Impossibile connettersi"), exit(-1);

  if (read(socketDesc, bufferRecieve, 1) < 0) {
    printf("impossibile leggere il messaggio\n");
  }
  printf("Ricevuto %d\n", bufferRecieve[0]);
  gestisci(bufferRecieve[0], socketDesc);

  close(socketDesc);
  exit(0);
}

int gestisci(int inputFromServer, int serverSocket) {
  char choice;
  int msg;
  switch (inputFromServer) {
  case 0:
    printMenu();
    choice = getUserInput();
    if (choice == '3') {
      printf("Uscita in corso\n");
      return (0);
    } else if (choice == '2') {
      msg = 2;
      write(serverSocket, &msg, sizeof(int));
      registrati(serverSocket);
      printf("Utente registrato con successso\n");
    } else if (choice == '1') {

    } else {
      printf("Wrong input");
      gestisci(inputFromServer, serverSocket);
    }
    break;

  default:
    break;
  }
}

char getUserInput() {
  char c;
  c = getchar();
  int daIgnorare;
  while ((daIgnorare = getchar()) != '\n' && daIgnorare != EOF) {
  }
  return c;
}
int login() { return 0; }

int registrati(int serverSocket) {
  char username[20];
  int dimUser, dimPass;
  char password[20];
  system("clear");
  printf("Inserisci nome utente(MAX 20 caratteri): ");
  dimUser = scanf("%s", username);
  printf("\nInserisci password(MAX 20 caratteri):");
  dimPass = scanf("%s", password);
  write(serverSocket, username, dimUser);
  write(serverSocket, password, dimPass);
  return 0;
}