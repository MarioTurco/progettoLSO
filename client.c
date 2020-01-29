#include "lib/boardUtility.h"
#include "lib/parser.h"
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

void play();
int tryLogin();
void printMenu();
int connettiAlServer(char **argv);
char *ipResolver(char **argv);
int registrati();
int gestisci();
char getUserInput();
int login();
void clientCrashHandler();
void serverCrashHandler();
int serverCaduto();
void esciDalServer();
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
  signal(SIGPIPE, serverCrashHandler);

  char bufferReceive[2];
  if (argc != 3)
    perror("Inserire indirizzo ip/url e porta (./client 127.0.0.1 5200)"),
        exit(-1);

  if ((socketDesc = connettiAlServer(argv)) < 0)
    exit(-1);

  gestisci(socketDesc);
  close(socketDesc);
  exit(0);
}
void esciDalServer() {
  int msg = 3;
  printf("Uscita in corso\n");
  write(socketDesc, &msg, sizeof(int));
  close(socketDesc);
}
int connettiAlServer(char **argv) {
  char *indirizzoServer;
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
    scanf("%c", &choice);
    system("clear");
    if (choice == '3') {
      esciDalServer();
      return (0);
    } else if (choice == '2') {
      registrati();
    } else if (choice == '1') {
      if (tryLogin()) {
        play();
      }
    } else {
      printf("Wrong input\n");
    }
  }
}

int serverCaduto() {
  char msg = 'y';
  if (read(socketDesc, &msg, sizeof(char)) == 0)
    return 1;
  else {
    msg = 'y';
    write(socketDesc, &msg, sizeof(msg));
  }
  return 0;
}
void play() {
  int exitFlag = 0;
  while (!exitFlag) {
    if (serverCaduto())
      serverCrashHandler();

    if (read(socketDesc, grigliaDiGioco, sizeof(grigliaDiGioco)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    printGrid(grigliaDiGioco);
  }
}

int tryLogin() {
  int msg = 1;
  write(socketDesc, &msg, sizeof(int));
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

  char validate;
  int ret;
  read(socketDesc, &validate, 1);
  if (validate == 'y') {
    ret = 1;
    printf("Accesso effettuato\n");
    premiEnterPerContinuare();
  } else if (validate == 'n') {
    printf("Credenziali Errate o Login già effettuato\n");
    ret = 0;
    premiEnterPerContinuare();
  }

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
  int msg = 2;
  write(socketDesc, &msg, sizeof(int));
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

  char validate;
  int ret;
  read(socketDesc, &validate, sizeof(char));
  if (validate == 'y') {
    ret = 1;
    printf("Registrato con successo\n");
    premiEnterPerContinuare();
  }
  if (validate == 'n') {
    ret = 0;
    printf("Registrazione fallita\n");
    premiEnterPerContinuare();
  }

  return ret;
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
  int rec = 0;
  printf("\nChiusura client...\n");
  do {
    write(socketDesc, &msg, sizeof(int));
    read(socketDesc, &rec, sizeof(int));
  } while (rec == 0);
  close(socketDesc);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  exit(0);
}
void serverCrashHandler() {
  system("clear");
  printf("Il server è crashato o è irraggiungibile\n");
  close(socketDesc);
  signal(SIGPIPE, SIG_IGN);
  premiEnterPerContinuare();
  exit(0);
}