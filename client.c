#include "boardUtility.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h> //conversioni
#include <netinet/in.h>
#include <netinet/ip.h> //struttura
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char grigliaDiGioco[ROWS][COLUMNS];
void printMenu();
int connettiAlServer(char **argv, char *indirizzoServer);
char *ipResolver(char **argv);
int registrati(int);
int gestisci(int, int);
char getUserInput();
int login();
int main(int argc, char **argv) {
  int socketDesc;
  char bufferRecieve[2];
  char *indirizzoServer;
  if (argc != 3)
    perror("Inserire indirizzo ip/url e porta (./client 127.0.0.1 5200)"),
        exit(-1);
  if ((socketDesc = connettiAlServer(argv, indirizzoServer)) < 0)
    exit(-1);
  if (read(socketDesc, bufferRecieve, 1) < 0) {
    printf("impossibile leggere il messaggio\n");
  }
  printf("Ricevuto %d\n", bufferRecieve[0]);
  gestisci(bufferRecieve[0], socketDesc);

  close(socketDesc);
  exit(0);
}
int connettiAlServer(char **argv, char *indirizzoServer) {
  int socketDesc;
  uint16_t porta = strtoul(argv[2], NULL, 10);
  indirizzoServer = ipResolver(argv);
  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(porta);
  inet_aton(indirizzoServer, &mio_indirizzo.sin_addr);
  if ((socketDesc = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    perror("Impossibile creare socket"), exit(-1);
  else
    printf("Socket creato\n");
  if (connect(socketDesc, (struct sockaddr *)&mio_indirizzo,
              sizeof(mio_indirizzo)) < 0)
    perror("Impossibile connettersi"), exit(-1);
  else
    printf("Connesso a %s\n", indirizzoServer);
  return socketDesc;
}

int gestisci(int inputFromServer, int serverSocket) {
  char choice;
  int msg;
  switch (inputFromServer) {
  case 0:
    printMenu();
    choice = getUserInput();
    system("clear");
    if (choice == '3') {
      printf("Uscita in corso\n");
      return (0);
    } else if (choice == '2') {
      msg = 2;
      write(serverSocket, &msg, sizeof(msg));
      if (registrati(serverSocket) < 0) {
        printf("Impossibile registrare Utente, riprovare");
        gestisci(inputFromServer, serverSocket);
        break;
      }
      printf("Utente registrato con successso\n");
    } else if (choice == '1') {
      msg = 1;
      write(serverSocket, &msg, sizeof(int));
      while (1) {
        read(serverSocket, grigliaDiGioco, sizeof(grigliaDiGioco));
        printGrid(grigliaDiGioco);
      }
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
  char password[20];
  system("clear");
  printf("Inserisci nome utente(MAX 20 caratteri): ");
  scanf("%s", username);
  printf("\nInserisci password(MAX 20 caratteri):");
  scanf("%s", password);
  int dimUname=strlen(username),dimPwd=strlen(password);
  if(write(serverSocket,&dimUname,sizeof(dimUname))<0)
    return 0;
  if(write(serverSocket,&dimPwd,sizeof(dimPwd))<0)
    return 0;
  if (write(serverSocket, username, dimUname) < 0)
    return 0;
  if (write(serverSocket, password, dimPwd) < 0)
    return 0;
  return 1;
}

void printMenu() {
  system("clear");
  printf("\t Cosa vuoi fare?\n");
  printf("\t1 Gioca\n");
  printf("\t2 Registrati\n");
  printf("\t3 Esci\n");
}

char *ipResolver(char **argv) {
  char *ipAddress;
  struct hostent *hp;
  hp = gethostbyname(argv[1]);
  if (!hp) {
    perror("Impossibile risolvere l'indirizzo ip\n");
    exit(-1);
  }
  printf("Address:\t%s\n", inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]));
  return inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]);
}