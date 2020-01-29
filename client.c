#include "boardUtility.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h> //conversioni
#include <netinet/in.h>
#include <netinet/ip.h> //struttura
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int tryLogin();
void printMenu();
int connettiAlServer(char **argv, char *indirizzoServer);
char *ipResolver(char **argv);
int registrati();
int gestisci();
char getUserInput();
int login();
void clientCrashHandler();
void serverCrashed();
/*/////////////////////////////*/
int socketDesc;
char grigliaDiGioco[ROWS][COLUMNS];
/*//////////////////////////////*/
int main(int argc, char **argv) {
  signal(SIGINT, clientCrashHandler); /* CTRL-C */
  signal(SIGHUP, clientCrashHandler); /* Chiusura della console */
  signal(SIGQUIT, clientCrashHandler);
  signal(SIGTSTP, clientCrashHandler); /* CTRL-Z*/
  signal(SIGTERM, clientCrashHandler); /* generato da 'kill' */
  signal(SIGPIPE, serverCrashed);
  char *indirizzoServer;

  char bufferReceive[2];
  if (argc != 3)
    perror("Inserire indirizzo ip/url e porta (./client 127.0.0.1 5200)"),
        exit(-1);

  if ((socketDesc = connettiAlServer(argv, indirizzoServer)) < 0)
    exit(-1);

  signal(SIGSTOP, clientCrashHandler);
  gestisci(socketDesc);
  close(socketDesc);
  exit(0);
}

int connettiAlServer(char **argv, char *indirizzoServer) {
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

int gestisci() {
  char choice;
  int msg;

  while (1) {
    printMenu();
    // choice = getUserInput();
    scanf("%c", &choice);
    system("clear");

    if (choice == '3') {
      printf("Uscita in corso\n");
      msg = 3;
      write(socketDesc, &msg, sizeof(int));
      return (0);
    }

    else if (choice == '2') {
      msg = 2;
      write(socketDesc, &msg, sizeof(int));
      if (!registrati(socketDesc)) {
        printf("Impossibile registrare Utente, riprovare");
      } else {
        printf("Utente registrato con successo\n");
      }
      sleep(2);
    }

    else if (choice == '1') {
      msg = 1;
      write(socketDesc, &msg, sizeof(int));
      char inputUtente;
      if (!tryLogin(socketDesc)) {
        printf("Credenziali Errate: riprova\n");
        sleep(2);
      } else {
        printf("Accesso effettuato\n");
        sleep(2);
        system("clear");
        while (1) {

          if (read(socketDesc, grigliaDiGioco, sizeof(grigliaDiGioco)) < 1)
            printf("Impossibile comunicare con il server\n"), exit(-1);
          printGrid(grigliaDiGioco);
          // inputUtente = getchar();
          // printf(" Input:%c\n", inputUtente);
        }
      }
    }

    else {
      printf("Wrong input\n");
    }
  }
}

int tryLogin() {
  system("clear");
  printf("Inserisci i dati per il Login\n");

  char username[20];
  char password[20];

  printf("Inserisci nome utente(MAX 20 caratteri): ");
  scanf("%s", username);
  printf("\nInserisci password(MAX 20 caratteri):");
  scanf("%s", password);

  int dimUname = strlen(username), dimPwd = strlen(password);

  if (write(socketDesc, &dimUname, sizeof(dimUname)) < 0)
    return 0;

  if (write(socketDesc, &dimPwd, sizeof(dimPwd)) < 0)
    return 0;

  if (write(socketDesc, username, dimUname) < 0)
    return 0;

  if (write(socketDesc, password, dimPwd) < 0)
    return 0;

  int ret = 0;

  read(socketDesc, &ret, sizeof(ret));

  return ret;
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

int registrati() {
  char username[20];
  char password[20];
  system("clear");
  printf("Inserisci nome utente(MAX 20 caratteri): ");
  scanf("%s", username);
  printf("\nInserisci password(MAX 20 caratteri):");
  scanf("%s", password);
  int dimUname = strlen(username), dimPwd = strlen(password);
  if (write(socketDesc, &dimUname, sizeof(dimUname)) < 0)
    return 0;
  if (write(socketDesc, &dimPwd, sizeof(dimPwd)) < 0)
    return 0;
  if (write(socketDesc, username, dimUname) < 0)
    return 0;
  if (write(socketDesc, password, dimPwd) < 0)
    return 0;

  int ret = 0;

  read(socketDesc, &ret, sizeof(ret));

  return ret;
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

void clientCrashHandler() {
  int msg = 3;
  write(socketDesc, &msg, sizeof(int));
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  exit(0);
}
void serverCrashed() {
  system("clear");
  printf("Il server è crashato o è irraggiungibile\n");
  signal(SIGPIPE, SIG_IGN);

  exit(0);
}