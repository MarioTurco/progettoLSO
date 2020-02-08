Progetto di sistemi operativi

Corso di Laurea Triennale in Informatica\
Professore:
Finzi Alberto 

Candidati:
Turco Mario
Matr. N8600/2503
Longobardi Francesco
Matr. N8600/2468

Anno Accademico
2019/2020

Istruzioni preliminari
======================

Modalità di compilazione
------------------------

#### 

Il progetto è provvisto di un file makefile il quale è in grado di
compilare autonomamente l'intero progetto. Per utilizzare il makefile
aprire la cartella del progetto tramite la console di sistema e digitare
\"make\".\
In alternativa è possibile compilare manualemente il client ed il server
con i seguenti comandi:

``` {fontsize="\\small"}
    gcc -o server server.c boardUtility.c parser.c list.c -lpthread
    gcc -o client client.c boardUtility.c parser.c list.c -lpthread
```

Guida all'uso 
=============

Server
------

#### 

Una volta compilato il progetto è possibile avviare il server digitando
da console il seguente comando

        ./server users

L'identificativo *users* ri riferisce al nome del file sul quale sarà
salvata la lista degli utenti e delle loro credenziali.\
È possibile scegliere un nome a piacimento per il file purchè esso sia
diverso da *log*.

Client
------

#### 

Una volta compilato il progetto è possibile avviare il cient digitando
da console il seguente comando:

        ./client ip porta

Dove *ip* andrà sostituito con l'ip o l'indirizzo URL del server e
*porta* andrà sostituito con la porta del server.\
Una volta avviato il client comprarià il menu con le scelte 3 possibili:
accedi, registrati ed esci.\
Una volta effettuata la registrazione dell'utente è possibile effettuare
l'accesso al programma al seguito del quale verranno mostrate sia la
mappa dei gioco si le istruzioni di gioco.

Comunicazione tra client e server
=================================

Di seguito verranno illustrate le modalità di comunicazione tra client e
server.

Configurazione del server
-------------------------

Il socket del server viene configurato con famiglia di protoccoli
PF\_NET, con tipo di trasmissione dati SOCK\_STREAM e con protocollo
TCP. Mostriamo di seguito il codice sorgente:

``` {.c caption="Configurazione indirizzo del server" firstline="173" lastline="181"}
struct sockaddr_in configuraIndirizzo()
{
  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  mio_indirizzo.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("Indirizzo socket configurato\n");
  return mio_indirizzo;
}
```

``` {.c caption="Configurazione socket del server" firstline="517" lastline="534"}
void configuraSocket(struct sockaddr_in mio_indirizzo)
{
  if ((socketDesc = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Impossibile creare socket");
    exit(-1);
  }
  if (setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <
      0)
    perror("Impossibile impostare il riutilizzo dell'indirizzo ip e della "
           "porta\n");
  if ((bind(socketDesc, (struct sockaddr *)&mio_indirizzo,
            sizeof(mio_indirizzo))) < 0)
  {
    perror("Impossibile effettuare bind");
    exit(-1);
  }
}
```

È importante notare anche come il server riesca a gestire in modo
concorrente più client tramite l'uso di un thread dedicato ad ogni
client. Una volta aver configurato il socket, infatti, il server si
mette in ascolto per nuove connessioni in entrata ed ogni volta che
viene stabilita una nuova connessione viene avviato un thread per
gestire tale connessione. Di seguito il relativo codice:

``` {.c caption="Procedura di ascolto del server" firstline="134" lastline="172"}
void startListening()
{
  pthread_t tid;
  int clientDesc;
  int *puntClientDesc;
  while (1 == 1)
  {
    if (listen(socketDesc, 10) < 0)
      perror("Impossibile mettersi in ascolto"), exit(-1);
    printf("In ascolto..\n");
    if ((clientDesc = accept(socketDesc, NULL, NULL)) < 0)
    {
      perror("Impossibile effettuare connessione\n");
      exit(-1);
    }
    printf("Nuovo client connesso\n");
    struct sockaddr_in address;
    socklen_t size = sizeof(struct sockaddr_in);
    if (getpeername(clientDesc, (struct sockaddr *)&address, &size) < 0)
    {
      perror("Impossibile ottenere l'indirizzo del client");
      exit(-1);
    }
    char clientAddr[20];
    strcpy(clientAddr, inet_ntoa(address.sin_addr));
    Args args = (Args)malloc(sizeof(struct argsToSend));
    args->userName = (char *)calloc(MAX_BUF, 1);
    strcpy(args->userName, clientAddr);
    args->flag = 2;
    pthread_t tid;
    pthread_create(&tid, NULL, fileWriter, (void *)args);

    puntClientDesc = (int *)malloc(sizeof(int));
    *puntClientDesc = clientDesc;
    pthread_create(&tid, NULL, gestisci, (void *)puntClientDesc);
  }
  close(clientDesc);
  quitServer();
}
```

In particolare al rigo 35 notiamo la creazione di un nuovo thread per
gestire la connessione in entrata a cui passiamo il descrittore del
client di cui si deve occupare. Dal rigo 19 al rigo 31, estraiamo invece
l'indirizzo ip del client per scriverlo sul file di log.

Configurazione del client
-------------------------

Il cliente invece viene configurato e si connette al server tramite la
seguente funzione:

``` {.c caption="Configurazione e connessione del client" firstline="65" lastline="83"}
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
```

Si noti come al rigo 9 viene configurato il socket ed al rigo 13 viene
invece effettuato il tentativo di connessione al server.\
Al rigo 3 invece viene convertita la porta inserita in input
(argv$[$2$]$) dal tipo stringa al tipo della porta (uint16\_t ovvero
unsigned long integer).\
Al rigo 4 notiamo invece la risoluzione dell'url da parte della funzione
ipResolver che è riportata di seguito:

``` {.c caption="Risoluzione url del client" firstline="250" lastline="261"}
char *ipResolver(char **argv) {
  char *ipAddress;
  struct hostent *hp;
  hp = gethostbyname(argv[1]);
  if (!hp) {
    perror("Impossibile risolvere l'indirizzo ip\n");
    sleep(1);
    exit(-1);
  }
  printf("Address:\t%s\n", inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]));
  return inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]);
}
```

Al rigo 4, tramite l'url o l'indirizzo ip viene riempita la struttura
hostent da cui poi possiamo estrarre l'indirizzo ip presente nel campo
h\_addr\_list che, in effetti, è un array che contiene i vari indirizzi
ip associati a quell'host.\
Infine, al rigo 11 decidiamo di ritoranre soltanto il primo indirizzo
convertito in Internet dot notation.

Comunicazione tra client e server
---------------------------------

La comunicazione tra client e server avviene tramite write e read sul
socket.\
Il comportamento del server e del client è determinato da particolari
messaggi inviati e/o ricevuti che codificano, tramite interi o
caratteri, la richiesta da parte del client di usufruire di un
determinato servizio e la relativa risposta del server.

### Esempio: la prima comunicazione

In particolare, una volta effettuata la connessione, il server attenderà
un messaggio dal client per poter avviare una delle tre possibili
procedure, ovvero login, registrazione ed uscita ( rispettivamente
codici: 1,2,3).\
Di seguito sono riportate le realtive funzioni di gestione che entrano
in esecuzione subito dopo aver stabilito la connessione tra client e
server.

``` {#alg:serverPreLogin .c caption="Prima comunicazione del server" label="alg:serverPreLogin" firstline="228" lastline="254"}
void *gestisci(void *descriptor)
{
  int bufferReceive[2] = {1};
  int client_sd = *(int *)descriptor;
  int continua = 1;
  char name[MAX_BUF];
  while (continua)
  {
    read(client_sd, bufferReceive, sizeof(bufferReceive));
    if (bufferReceive[0] == 2)
      registraClient(client_sd);
    else if (bufferReceive[0] == 1)
      if (tryLogin(client_sd, name))
      {
        play(client_sd, name);
        continua = 0;
      }
      else if (bufferReceive[0] == 3)
        disconnettiClient(client_sd);
      else
      {
        printf("Input invalido, uscita...\n");
        disconnettiClient(client_sd);
      }
  }
  pthread_exit(0);
}
```

Si noti come il server riceva, al rigo 9, il messaggio codificato da
parte del client e metta in esecuzione la funzione corrispondente.

``` {#alg:clientPreLogin .c caption="Prima comunicazione del client" firstline="84" lastline="102" label="alg:clientPreLogin"}
int gestisci() {
  char choice;
  while (1) {
    printMenu();
    scanf("%c", &choice);
    fflush(stdin);
    system("clear");
    if (choice == '3') {
      esciDalServer();
      return (0);
    } else if (choice == '2') {
      registrati();
    } else if (choice == '1') {
      if (tryLogin())
        play();
    } else
      printf("Input errato, inserire 1,2 o 3\n");
  }
}
```

Comunicazione durante la partita
================================

#### 

Una volta effettuato il login, il client potrà iniziare a giocare
tramite la funzione play (Vedi e ) che rappresentano il cuore della
comunicazione tra client e server.

Funzione core del server
------------------------

La funzione play del server consiste di un ciclo nel quale il server
invia al client tre informazioni importanti:

-   La griglia di gioco (Rigo 32)

-   Il player con le relative informazioni (Righi 34 a 37)

-   Un messaggio che notifica al client se è iniziato un nuovo turno
    oppure no (Righi 52,57,62,71)

Dopodichè il thread del server rimane in attesa di ricevere l'input del
client per spostare il giocaore sulla mappa tramite la relativa
funzione. (Rigo 33) (Vedi Rigo 430 e Rigo 296, 331,367, 405 ) Oltre
questo, la funzione play del server si occupa anche di generare la
posizione del player appena entra in partita, generare la nuova
posizione (Righi 56 a 65) quando viene effettuato il cambio di mappa ed
inviare il tempo rimanente o la lista degli utente loggati su richiesta
del client.

#### 

È anche importante notare il seguente dettaglio implementativo: la
griglia di gioco è una matrice globale definita nel file del server che
contiene tutti i player, i punti di raccolta ed i pacchi, mentre gli
ostacoli sono contenuti in una seconda matrice globale del server. Ogni
client però deve vedere soltanto gli ostacoli che ha già scoperto, per
questo motivo ad ogni client non viene mandata direttamente la matrice
di gioco, bensì, dai righi 22 a 24, inizializziamo una nuova matrice
temporanea a cui aggiungiamo gli ostacoli già scoperti dal client (rigo
24) prima di mandarla al client stesso.\
In questo modo ci assicuriamo che ogni client visualizzi soltanto gli
ostacoli che ha già scoperto.

``` {#alg:playServer .c caption="Funzione play del server" label="alg:playServer" firstline="255" lastline="342"}
void play(int clientDesc, char name[])
{
  int true = 1;
  int turnoFinito = 0;
  int turnoGiocatore = turno;
  int posizione[2];
  int destinazione[2] = {-1, -1};
  PlayerStats giocatore = initStats(destinazione, 0, posizione, 0);
  Obstacles listaOstacoli = NULL;
  char inputFromClient;
  if (timer != 0)
  {
    inserisciPlayerNellaGrigliaInPosizioneCasuale(
        grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
        giocatore->position);
    pthread_mutex_lock(&PlayerGeneratiMutex);
    playerGenerati++;
    pthread_mutex_unlock(&PlayerGeneratiMutex);
  }
  while (true)
  {
    if (clientDisconnesso(clientDesc))
    {
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
    // legge l'input
    if (read(clientDesc, &inputFromClient, sizeof(char)) > 0){
      pthread_mutex_lock(&numMosseMutex);
      numMosse++;
      pthread_mutex_unlock(&numMosseMutex);
    }
    if (inputFromClient == 'e' || inputFromClient == 'E')
    {
      freeObstacles(listaOstacoli);
      listaOstacoli = NULL;
      disconnettiClient(clientDesc);
    }
    else if (inputFromClient == 't' || inputFromClient == 'T')
    {
      write(clientDesc, &turnoFinito, sizeof(int));
      sendTimerValue(clientDesc);
    }
    else if (inputFromClient == 'l' || inputFromClient == 'L')
    {
      write(clientDesc, &turnoFinito, sizeof(int));
      sendPlayerList(clientDesc);
    }
    else if (turnoGiocatore == turno)
    {
      write(clientDesc, &turnoFinito, sizeof(int));
      giocatore =
          gestisciInput(grigliaDiGiocoConPacchiSenzaOstacoli,
                        grigliaOstacoliSenzaPacchi, inputFromClient, giocatore,
                        &listaOstacoli, deployCoords, packsCoords, name);
    }
    else
    {
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
      pthread_mutex_lock(&PlayerGeneratiMutex);
      playerGenerati++;
      pthread_mutex_unlock(&PlayerGeneratiMutex);
    }
  }
}
```

Funzione core del client
------------------------

Dall'altro lato, la funzione play del client, è stata mantenuta il puù
semplice possibile. Lo scopo del client è unicamente quello di ricevere
i dati forniti dal server, stampare la mappa di gioco e ed inviare un
input al server che rappresenta la volontà del giocatore di muoversi,
vedere la lista degli utenti, uscire o stampare il timer. Unica
eccezzione è il rigo 30 del client che non richiede la ricezione di
ulteriori dati dal server: al rigo 23, infatti si avvia la procedura di
disconnessione del client (Vedi rigo 59).

``` {#alg:playclient .c caption="Funzione play del client" label="alg:playclient" firstline="111" lastline="149"}
void play() {
  PlayerStats giocatore = NULL;
  int score, deploy[2], position[2], timer;
  int turnoFinito = 0;
  int exitFlag = 0, hasApack = 0;
  while (!exitFlag) {
    if (serverCaduto())
      serverCrashHandler();
    if (read(socketDesc, grigliaDiGioco, sizeof(grigliaDiGioco)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, deploy, sizeof(deploy)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, position, sizeof(position)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, &score, sizeof(score)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, &hasApack, sizeof(hasApack)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    giocatore = initStats(deploy, score, position, hasApack);
    printGrid(grigliaDiGioco, giocatore);
    char send = getUserInput();
    if (send == 'e' || send == 'E') {
      esciDalServer();
      exit(0);
    }
    write(socketDesc, &send, sizeof(char));
    read(socketDesc, &turnoFinito, sizeof(turnoFinito));
    if (turnoFinito) {
      system("clear");
      printf("Turno finito\n");
      sleep(1);
    } else {
      if (send == 't' || send == 'T')
        printTimer();
      else if (send == 'l' || send == 'L')
        printPlayerList();
    }
  }
}
```

Dettagli implementativi degni di nota
=====================================

In questa sezione verranno trattati alcuni dettagli implementativi da
noi giudicati interessanti in realzione a ciò che è stato studiato
durante il corso di sistemi operativi.

Timer
-----

#### 

Lo svolgimento della partite è legato al timer: ogni round durerà un
numero finito di secondi od oppure terminarà quando un client
raccoglierà il numero massimo di pacchi.

#### 

Subito dopo aver configurato il socket, il server inizia la procedura di
avvio del timer (Vedi rigo 126 e 187) che farà partire un thread il
quale si occuperà di decrementare e resettare correttamente il timer
(definito come variabile globale del server).

``` {.c caption="Funzione di gestione del timer" firstline="478" lastline="515"}
void *timer(void *args)
{
  int cambiato = 1;
  while (1)
  {
    if (almenoUnClientConnesso() && valoreTimerValido() &&
        almenoUnPlayerGenerato() && almenoUnaMossaFatta())
    {
      cambiato = 1;
      sleep(1);
      timerCount--;
      fprintf(stdout, "Time left: %d\n", timerCount);
    }
    else if (numeroClientLoggati == 0)
    {
      timerCount = TIME_LIMIT_IN_SECONDS;
      if (cambiato)
      {
        fprintf(stdout, "Time left: %d\n", timerCount);
        cambiato = 0;
      }
    }
    if (timerCount == 0 || scoreMassimo == packageLimitNumber)
    {
      pthread_mutex_lock(&PlayerGeneratiMutex);
      playerGenerati = 0;
      pthread_mutex_unlock(&PlayerGeneratiMutex);
      pthread_mutex_lock(&numMosseMutex);
      numMosse = 0;
      pthread_mutex_unlock(&numMosseMutex);
      printf("Reset timer e generazione nuova mappa..\n");
      startProceduraGenrazioneMappa();
      pthread_join(tidGeneratoreMappa, NULL);
      turno++;
      timerCount = TIME_LIMIT_IN_SECONDS;
    }
  }
}
```

Analizzando il codice della funzione di modifica del timer si può notare
un dettaglio abbastanza interessante: il thread che esegue la funzione
del timer è legato ad un altro thread, ovvero quello della generazione
di una nuova mappa. Oltre ad un thread per gestire ogni client abbiamo
quindi anche un altro thread che va a gestire il tempo , il quale
attraverso un altro thread riesce a controllare la generazione della
mappa e degli utenti allo scadere del tempo. Si noti anche come, tramite
il pthread\_join, il timer attenda la terminazione del secondo thread
prima di resettare il timer e ricominciare il contro alla rovescia. [^1]

#### 

Per completezza si riporta anche la funzionione iniziale del thread di
generazione mappa

``` {.c caption="Generazione nuova mappa e posizione players" firstline="441" lastline="453"}
void *threadGenerazioneMappa(void *args)
{
  fprintf(stdout, "Rigenerazione mappa\n");
  inizializzaGrigliaVuota(grigliaDiGiocoConPacchiSenzaOstacoli);
  generaPosizioniRaccolta(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi, deployCoords);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
      grigliaDiGiocoConPacchiSenzaOstacoli, packsCoords);
  generaPosizioneOstacoli(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi);
  printf("Mappa generata\n");
  pthread_exit(NULL);
}
```

Gestione del file di Log
------------------------

#### 

Una delle funzionalità del server è quella di creare un file di log con
varie informazioni durante la sua esecuzione. Riteniamo l'implmentazione
di questa funzione piuttosto interessante poichè, oltre ad essere una
funzione gestita tramite un thread, fa uso sia di molte chiamate di
sistema studiate durante il corso ed utilizza anche il mutex per
risolvere eventuali race condition. Riportiamo di seguito il codice:

``` {.c caption="Funzione di log" firstline="683" lastline="721"}
void *fileWriter(void *args)
{
  int fDes = open("Log", O_RDWR | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);
  if (fDes < 0)
  {
    perror("Error while opening log file");
    exit(-1);
  }
  Args info = (Args)args;
  char dateAndTime[64];
  putCurrentDateAndTimeInString(dateAndTime);
  if (logDelPacco(info->flag))
  {
    char message[MAX_BUF] = "";
    prepareMessageForPackDelivery(message, info->userName, dateAndTime);
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  }
  else if (logDelLogin(info->flag))
  {
    char message[MAX_BUF] = "\"";
    prepareMessageForLogin(message, info->userName, dateAndTime);
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  }
  else if (logDellaConnessione(info->flag))
  {
    char message[MAX_BUF] = "\"";
    prepareMessageForConnection(message, info->userName, dateAndTime);
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  }
  close(fDes);
  free(info);
  pthread_exit(NULL);
}
```

#### 

Analizzando il codice si può notare l'uso open per aprire in append o,
in caso di assenza del file, di creare il file di log ed i vari write
per scrivere sul suddetto file; possiamo anche notare come la sezione
critica, ovvero la scrittura su uno stesso file da parte di più thread,
è gestita tramite un mutex.

Modifica della mappa di gioco da parte di più thread
----------------------------------------------------

La mappa di gioco è la stessa per tutti i player e c'è il rischio che lo
spostamento dei player e/o la raccolta ed il deposito di pacchi possano
provocare problemi a causa della race condition che si viene a creare
tra i thread. Tutto ciò è stato risolto con una serie di semplici
accorgimenti implementativi. Il primo accorgimento, e forse anche il più
importante, è la funzione spostaPlayer mostrata qui di seguito.

``` {.c caption="Funzione spostaPlayer" firstline="723" lastline="737"}
void spostaPlayer(char griglia[ROWS][COLUMNS], int vecchiaPosizione[2],
                  int nuovaPosizione[2], Point deployCoords[],
                  Point packsCoords[])
{

  pthread_mutex_lock(&MatrixMutex);
  griglia[nuovaPosizione[0]][nuovaPosizione[1]] = 'P';
  if (eraUnPuntoDepo(vecchiaPosizione, deployCoords))
    griglia[vecchiaPosizione[0]][vecchiaPosizione[1]] = '_';
  else if (eraUnPacco(vecchiaPosizione, packsCoords))
    griglia[vecchiaPosizione[0]][vecchiaPosizione[1]] = '$';
  else
    griglia[vecchiaPosizione[0]][vecchiaPosizione[1]] = '-';
  pthread_mutex_unlock(&MatrixMutex);
}
```

Questa funzione rappresenta l'unico punto del programma che
effettivamente modifica la matrice di gioco in seguito ad una richiesta
di un client. È possibile notare come l'intera funzione sia racchiusa in
un mutex in modo da evitare che contemporaneamente più thread
modifichino la mappa di gioco e quindi evita che due player si trovino
nella stessa poszione.

#### 

Il secondo accorgimento è stato quello di far in modo che un player
possa raccogliere un pacco solo quando si trova nella poszione del pacco
(\"sia sovrapposto al pacco\") e possa depositare un pacco solo nella
posizione in cui il player stesso si trova (\"deposita il pacco su se
stesso\").

#### 

Questi due accorgimenti, assieme, evitano qualsiasi tipo di conflitto
tra i player: due player non potranno mai trovarsi nella stessa
posizione e, di conseguenza non potranno mai raccogliere lo stesso pacco
o depositare due pacchi nella stessa posizione contemporaneamente.

Gestione del login
------------------

La gestione del login è il quarto ed ultimo dettagli implementativo
giusdicato abbastanza interessante poichè fa uso della system call
system() per utilizzare le chiamate di sistema unix studiate durante la
prima parte del corso. Di seguito riportiamo il codice e la spiegazione

``` {.c caption="\"Gestion del login 1\"" firstline="34" lastline="52"}
int isRegistered(char *name, char *file) {
  char command[MAX_BUF] = "cat  ";
  strcat(command, file);
  char toApp[] = " |cut -d\" \" -f1|grep \"^";
  strcat(command, toApp);
  strcat(command, name);
  char toApp2[] = "$\">tmp";
  strcat(command, toApp2);
  int ret = 0;
  system(command);
  int fileDes = openFileRDON("tmp");
  struct stat info;
  fstat(fileDes, &info);
  if ((int)info.st_size > 0)
    ret = 1;
  close(fileDes);
  system("rm tmp");
  return ret;
}
```

La funzione isRegistered tramite varie concatenazioni produce ed esegue
il seguente comando

      cat file | cut -d" " -f1|grep "^name$">tmp

Ovvero andiamo a leggere la prima colonna (dove sono conservati tutti i
nomi utente) dal file degli utenti registrati, cerchiamo la stringa che
combacia esattamente con name e la scriviamo sul file temporaneo
\"tmp\".\
Dopodichè andiamo a verificare la dimensione del file tmp tramite la
struttura stat: se la dimensione è maggiore di 0 allora significa che è
il nome esisteva nella lista dei client registrati ed è stato quindi
trascritto in tmp altrimenti significa che il nome non era presente
nella lista dei player registrati. A questo punto eliminiamo il file
temporaneo e restituiamo il valore appropriato.

#### 

``` {.c caption="\"Gestion del login 2\"" firstline="53" lastline="75"}
int validateLogin(char *name, char *pwd, char *file) {
  if (!isRegistered(name, file))
    return 0;
  char command[MAX_BUF] = "cat  ";
  strcat(command, file);
  char toApp[] = " |grep \"^";
  strcat(command, toApp);
  strcat(command, name);
  strcat(command, " ");
  strcat(command, pwd);
  char toApp2[] = "$\">tmp";
  strcat(command, toApp2);
  int ret = 0;
  system(command);
  int fileDes = openFileRDON("tmp");
  struct stat info;
  fstat(fileDes, &info);
  if ((int)info.st_size > 0)
    ret = 1;
  close(fileDes);
  system("rm tmp");
  return ret;
}
```

La funziona validateLogin invece, tramite concatenazioni successive crea
ed esegue il seguente comando:

      cat file | grep "^nome password$">tmp

Verificando se la coppia nome password sia presente nel file degli
utenti registrati, trascrivendola sul file tmp Ancora una volta si va a
verificare tramite la struttura stat se è stato trascritto qualcosa
oppure no, ritornando il valore appropriato.

Codici sorgente
===============

Di seguito sono riportati tutti i codici sorgenti integrali del
progetto.

Codice sorgente del client
--------------------------

``` {#alg:sorgenteClient .c caption="Codice sorgente del client" label="alg:sorgenteClient"}
#include "boardUtility.h"
#include "list.h"
#include "parser.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h> //conversioni
#include <netinet/in.h>
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

void printPlayerList();
int getTimer();
void printTimer();
void play();
int tryLogin();
void printMenu();
int connettiAlServer(char **argv);
char *ipResolver(char **argv);
int registrati();
int gestisci();
char getUserInput();
void clientCrashHandler();
void serverCrashHandler();
int serverCaduto();
void esciDalServer();
int isCorrect(char);

int socketDesc;
char grigliaDiGioco[ROWS][COLUMNS];

int main(int argc, char **argv) {
  signal(SIGINT, clientCrashHandler); /* CTRL-C */
  signal(SIGHUP, clientCrashHandler); /* Chiusura della console */
  signal(SIGQUIT, clientCrashHandler);
  signal(SIGTSTP, clientCrashHandler); /* CTRL-Z*/
  signal(SIGTERM, clientCrashHandler); /* generato da 'kill' */
  signal(SIGPIPE, serverCrashHandler);
  char bufferReceive[2];
  if (argc != 3) {
    perror("Inserire indirizzo ip/url e porta (./client 127.0.0.1 5200)");
    exit(-1);
  }
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
  while (1) {
    printMenu();
    scanf("%c", &choice);
    fflush(stdin);
    system("clear");
    if (choice == '3') {
      esciDalServer();
      return (0);
    } else if (choice == '2') {
      registrati();
    } else if (choice == '1') {
      if (tryLogin())
        play();
    } else
      printf("Input errato, inserire 1,2 o 3\n");
  }
}
int serverCaduto() {
  char msg = 'y';
  if (read(socketDesc, &msg, sizeof(char)) == 0)
    return 1;
  else
    write(socketDesc, &msg, sizeof(msg));
  return 0;
}
void play() {
  PlayerStats giocatore = NULL;
  int score, deploy[2], position[2], timer;
  int turnoFinito = 0;
  int exitFlag = 0, hasApack = 0;
  while (!exitFlag) {
    if (serverCaduto())
      serverCrashHandler();
    if (read(socketDesc, grigliaDiGioco, sizeof(grigliaDiGioco)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, deploy, sizeof(deploy)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, position, sizeof(position)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, &score, sizeof(score)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    if (read(socketDesc, &hasApack, sizeof(hasApack)) < 1)
      printf("Impossibile comunicare con il server\n"), exit(-1);
    giocatore = initStats(deploy, score, position, hasApack);
    printGrid(grigliaDiGioco, giocatore);
    char send = getUserInput();
    if (send == 'e' || send == 'E') {
      esciDalServer();
      exit(0);
    }
    write(socketDesc, &send, sizeof(char));
    read(socketDesc, &turnoFinito, sizeof(turnoFinito));
    if (turnoFinito) {
      system("clear");
      printf("Turno finito\n");
      sleep(1);
    } else {
      if (send == 't' || send == 'T')
        printTimer();
      else if (send == 'l' || send == 'L')
        printPlayerList();
    }
  }
}
void printPlayerList() {
  system("clear");
  int lunghezza = 0;
  char buffer[100];
  int continua = 1;
  int number = 1;
  fprintf(stdout, "Lista dei player: \n");
  if (!serverCaduto(socketDesc)) {
    read(socketDesc, &continua, sizeof(continua));
    while (continua) {
      read(socketDesc, &lunghezza, sizeof(lunghezza));
      read(socketDesc, buffer, lunghezza);
      buffer[lunghezza] = '\0';
      fprintf(stdout, "%d) %s\n", number, buffer);
      continua--;
      number++;
    }
    sleep(1);
  }
}
void printTimer() {
  int timer;
  if (!serverCaduto(socketDesc)) {
    read(socketDesc, &timer, sizeof(timer));
    printf("\t\tTempo restante: %d...\n", timer);
    sleep(1);
  }
}
int getTimer() {
  int timer;
  if (!serverCaduto(socketDesc))
    read(socketDesc, &timer, sizeof(timer));
  return timer;
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
  } else if (validate == 'n') {
    printf("Credenziali Errate o Login già effettuato\n");
    ret = 0;
  }
  sleep(1);
  return ret;
}
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
  }
  if (validate == 'n') {
    ret = 0;
    printf("Registrazione fallita\n");
  }
  sleep(1);
  return ret;
}
char *ipResolver(char **argv) {
  char *ipAddress;
  struct hostent *hp;
  hp = gethostbyname(argv[1]);
  if (!hp) {
    perror("Impossibile risolvere l'indirizzo ip\n");
    sleep(1);
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
  printf("Il server è stato spento o è irraggiungibile\n");
  close(socketDesc);
  signal(SIGPIPE, SIG_IGN);
  premiEnterPerContinuare();
  exit(0);
}
char getUserInput() {
  char c;
  c = getchar();
  int daIgnorare;
  while ((daIgnorare = getchar()) != '\n' && daIgnorare != EOF) {
  }
  return c;
}
```

Codice sorgente del server
--------------------------

``` {#alg:sorgenteServer .c caption="Codice sorgente del server" label="alg:sorgenteServer"}
#include "boardUtility.h"
#include "list.h"
#include "parser.h"
#include <arpa/inet.h>
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

//struttura di argomenti da mandare al thread che scrive sul file di log
struct argsToSend
{
  char *userName;
  int flag;
};

typedef struct argsToSend *Args;
void prepareMessageForLogin(char message[], char username[], char date[]);
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
void prepareMessageForConnection(char message[], char ipAddress[], char date[]);
void spostaPlayer(char griglia[ROWS][COLUMNS], int vecchiaPosizione[2],
                  int nuovaPosizione[2], Point deployCoords[],
                  Point packsCoords[]);
int valoreTimerValido();
PlayerStats gestisciW(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[]);
PlayerStats gestisciA(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[]);
PlayerStats gestisciD(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[]);
PlayerStats gestisciS(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[]);
int almenoUnPlayerGenerato();
int almenoUnaMossaFatta();
void sendTimerValue(int clientDesc);
void putCurrentDateAndTimeInString(char dateAndTime[]);
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
void prepareMessageForPackDelivery(char message[], char username[], char date[]);
int logDelPacco(int flag);
int logDelLogin(int flag);
int logDellaConnessione(int flag);

char grigliaDiGiocoConPacchiSenzaOstacoli[ROWS][COLUMNS]; //protetta
char grigliaOstacoliSenzaPacchi[ROWS][COLUMNS];           //protetta
int numeroClientLoggati = 0;                              //protetto
int playerGenerati = 0;                                   //mutex
int timerCount = TIME_LIMIT_IN_SECONDS;
int turno = 0; //lo cambia solo timer
pthread_t tidTimer;
pthread_t tidGeneratoreMappa;
int socketDesc;
Players onLineUsers = NULL; //protetto
char *users;
int scoreMassimo = 0; //mutex
int numMosse = 0;     //mutex
Point deployCoords[numberOfPackages];
Point packsCoords[numberOfPackages];
pthread_mutex_t LogMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t RegMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PlayerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MatrixMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PlayerGeneratiMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ScoreMassimoMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t numMosseMutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("Wrong parameters number(Usage: ./server usersFile)\n");
    exit(-1);
  }
  else if (strcmp(argv[1], "Log") == 0)
  {
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
void startListening()
{
  pthread_t tid;
  int clientDesc;
  int *puntClientDesc;
  while (1 == 1)
  {
    if (listen(socketDesc, 10) < 0)
      perror("Impossibile mettersi in ascolto"), exit(-1);
    printf("In ascolto..\n");
    if ((clientDesc = accept(socketDesc, NULL, NULL)) < 0)
    {
      perror("Impossibile effettuare connessione\n");
      exit(-1);
    }
    printf("Nuovo client connesso\n");
    struct sockaddr_in address;
    socklen_t size = sizeof(struct sockaddr_in);
    if (getpeername(clientDesc, (struct sockaddr *)&address, &size) < 0)
    {
      perror("Impossibile ottenere l'indirizzo del client");
      exit(-1);
    }
    char clientAddr[20];
    strcpy(clientAddr, inet_ntoa(address.sin_addr));
    Args args = (Args)malloc(sizeof(struct argsToSend));
    args->userName = (char *)calloc(MAX_BUF, 1);
    strcpy(args->userName, clientAddr);
    args->flag = 2;
    pthread_t tid;
    pthread_create(&tid, NULL, fileWriter, (void *)args);

    puntClientDesc = (int *)malloc(sizeof(int));
    *puntClientDesc = clientDesc;
    pthread_create(&tid, NULL, gestisci, (void *)puntClientDesc);
  }
  close(clientDesc);
  quitServer();
}
struct sockaddr_in configuraIndirizzo()
{
  struct sockaddr_in mio_indirizzo;
  mio_indirizzo.sin_family = AF_INET;
  mio_indirizzo.sin_port = htons(5200);
  mio_indirizzo.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("Indirizzo socket configurato\n");
  return mio_indirizzo;
}
void startProceduraGenrazioneMappa()
{
  printf("Inizio procedura generazione mappa\n");
  pthread_create(&tidGeneratoreMappa, NULL, threadGenerazioneMappa, NULL);
}
void startTimer()
{
  printf("Thread timer avviato\n");
  pthread_create(&tidTimer, NULL, timer, NULL);
}
int tryLogin(int clientDesc, char name[])
{
  char *userName = (char *)calloc(MAX_BUF, 1);
  char *password = (char *)calloc(MAX_BUF, 1);
  int dimName, dimPwd;
  read(clientDesc, &dimName, sizeof(int));
  read(clientDesc, &dimPwd, sizeof(int));
  read(clientDesc, userName, dimName);
  read(clientDesc, password, dimPwd);
  int ret = 0;
  pthread_mutex_lock(&PlayerMutex);
  if (validateLogin(userName, password, users) &&
      !isAlreadyLogged(onLineUsers, userName))
  {
    ret = 1; 
    write(clientDesc, "y", 1);
    strcpy(name, userName);
    Args args = (Args)malloc(sizeof(struct argsToSend));
    args->userName = (char *)calloc(MAX_BUF, 1);
    strcpy(args->userName, name);
    args->flag = 0;
    pthread_t tid;
    pthread_create(&tid, NULL, fileWriter, (void *)args);
    printf("Nuovo client loggato, client loggati : %d\n", numeroClientLoggati);
    onLineUsers = addPlayer(onLineUsers, userName, clientDesc);
    numeroClientLoggati++;
    pthread_mutex_unlock(&PlayerMutex);
    printPlayers(onLineUsers);
    printf("\n");
  }
  else
  {
    write(clientDesc, "n", 1);
  }
  return ret;
}
void *gestisci(void *descriptor)
{
  int bufferReceive[2] = {1};
  int client_sd = *(int *)descriptor;
  int continua = 1;
  char name[MAX_BUF];
  while (continua)
  {
    read(client_sd, bufferReceive, sizeof(bufferReceive));
    if (bufferReceive[0] == 2)
      registraClient(client_sd);
    else if (bufferReceive[0] == 1)
      if (tryLogin(client_sd, name))
      {
        play(client_sd, name);
        continua = 0;
      }
      else if (bufferReceive[0] == 3)
        disconnettiClient(client_sd);
      else
      {
        printf("Input invalido, uscita...\n");
        disconnettiClient(client_sd);
      }
  }
  pthread_exit(0);
}
void play(int clientDesc, char name[])
{
  int true = 1;
  int turnoFinito = 0;
  int turnoGiocatore = turno;
  int posizione[2];
  int destinazione[2] = {-1, -1};
  PlayerStats giocatore = initStats(destinazione, 0, posizione, 0);
  Obstacles listaOstacoli = NULL;
  char inputFromClient;
  if (timer != 0)
  {
    inserisciPlayerNellaGrigliaInPosizioneCasuale(
        grigliaDiGiocoConPacchiSenzaOstacoli, grigliaOstacoliSenzaPacchi,
        giocatore->position);
    pthread_mutex_lock(&PlayerGeneratiMutex);
    playerGenerati++;
    pthread_mutex_unlock(&PlayerGeneratiMutex);
  }
  while (true)
  {
    if (clientDisconnesso(clientDesc))
    {
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
    // legge l'input
    if (read(clientDesc, &inputFromClient, sizeof(char)) > 0){
      pthread_mutex_lock(&numMosseMutex);
      numMosse++;
      pthread_mutex_unlock(&numMosseMutex);
    }
    if (inputFromClient == 'e' || inputFromClient == 'E')
    {
      freeObstacles(listaOstacoli);
      listaOstacoli = NULL;
      disconnettiClient(clientDesc);
    }
    else if (inputFromClient == 't' || inputFromClient == 'T')
    {
      write(clientDesc, &turnoFinito, sizeof(int));
      sendTimerValue(clientDesc);
    }
    else if (inputFromClient == 'l' || inputFromClient == 'L')
    {
      write(clientDesc, &turnoFinito, sizeof(int));
      sendPlayerList(clientDesc);
    }
    else if (turnoGiocatore == turno)
    {
      write(clientDesc, &turnoFinito, sizeof(int));
      giocatore =
          gestisciInput(grigliaDiGiocoConPacchiSenzaOstacoli,
                        grigliaOstacoliSenzaPacchi, inputFromClient, giocatore,
                        &listaOstacoli, deployCoords, packsCoords, name);
    }
    else
    {
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
      pthread_mutex_lock(&PlayerGeneratiMutex);
      playerGenerati++;
      pthread_mutex_unlock(&PlayerGeneratiMutex);
    }
  }
}
void sendTimerValue(int clientDesc)
{
  if (!clientDisconnesso(clientDesc))
    write(clientDesc, &timerCount, sizeof(timerCount));
}
void clonaGriglia(char destinazione[ROWS][COLUMNS],
                  char source[ROWS][COLUMNS])
{
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++)
  {
    for (j = 0; j < COLUMNS; j++)
    {
      destinazione[i][j] = source[i][j];
    }
  }
}
void clientCrashHandler(int signalNum)
{
  char msg[0];
  int socketClientCrashato;
  int flag = 1;
  // TODO eliminare la lista degli ostacoli dell'utente
  if (onLineUsers != NULL)
  {
    Players prec = onLineUsers;
    Players top = prec->next;
    while (top != NULL && flag)
    {
      if (write(top->sockDes, msg, sizeof(msg)) < 0)
      {
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
void disconnettiClient(int clientDescriptor)
{
  pthread_mutex_lock(&PlayerMutex);
  if (numeroClientLoggati > 0)
    numeroClientLoggati--;
  onLineUsers = removePlayer(onLineUsers, clientDescriptor);
  pthread_mutex_unlock(&PlayerMutex);
  printPlayers(onLineUsers);
  int msg = 1;
  printf("Client disconnesso (client attualmente loggati: %d)\n",
         numeroClientLoggati);
  write(clientDescriptor, &msg, sizeof(msg));
  close(clientDescriptor);
}
int clientDisconnesso(int clientSocket)
{
  char msg[1] = {'u'}; // UP?
  if (write(clientSocket, msg, sizeof(msg)) < 0)
    return 1;
  if (read(clientSocket, msg, sizeof(char)) < 0)
    return 1;
  else
    return 0;
}
int registraClient(int clientDesc)
{
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
  if (!ret)
  {
    risposta = 'n';
    write(clientDesc, &risposta, sizeof(char));
    printf("Impossibile registrare utente, riprovare\n");
  }
  else
  {
    risposta = 'y';
    write(clientDesc, &risposta, sizeof(char));
    printf("Utente registrato con successo\n");
  }
  return ret;
}
void quitServer()
{
  printf("Chiusura server in corso..\n");
  close(socketDesc);
  exit(-1);
}
void *threadGenerazioneMappa(void *args)
{
  fprintf(stdout, "Rigenerazione mappa\n");
  inizializzaGrigliaVuota(grigliaDiGiocoConPacchiSenzaOstacoli);
  generaPosizioniRaccolta(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi, deployCoords);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
      grigliaDiGiocoConPacchiSenzaOstacoli, packsCoords);
  generaPosizioneOstacoli(grigliaDiGiocoConPacchiSenzaOstacoli,
                          grigliaOstacoliSenzaPacchi);
  printf("Mappa generata\n");
  pthread_exit(NULL);
}
int almenoUnaMossaFatta()
{
  if (numMosse > 0)
    return 1;
  return 0;
}
int almenoUnClientConnesso()
{
  if (numeroClientLoggati > 0)
    return 1;
  return 0;
}
int valoreTimerValido()
{
  if (timerCount > 0 && timerCount <= TIME_LIMIT_IN_SECONDS)
    return 1;
  return 0;
}
int almenoUnPlayerGenerato()
{
  if (playerGenerati > 0)
    return 1;
  return 0;
}
void *timer(void *args)
{
  int cambiato = 1;
  while (1)
  {
    if (almenoUnClientConnesso() && valoreTimerValido() &&
        almenoUnPlayerGenerato() && almenoUnaMossaFatta())
    {
      cambiato = 1;
      sleep(1);
      timerCount--;
      fprintf(stdout, "Time left: %d\n", timerCount);
    }
    else if (numeroClientLoggati == 0)
    {
      timerCount = TIME_LIMIT_IN_SECONDS;
      if (cambiato)
      {
        fprintf(stdout, "Time left: %d\n", timerCount);
        cambiato = 0;
      }
    }
    if (timerCount == 0 || scoreMassimo == packageLimitNumber)
    {
      pthread_mutex_lock(&PlayerGeneratiMutex);
      playerGenerati = 0;
      pthread_mutex_unlock(&PlayerGeneratiMutex);
      pthread_mutex_lock(&numMosseMutex);
      numMosse = 0;
      pthread_mutex_unlock(&numMosseMutex);
      printf("Reset timer e generazione nuova mappa..\n");
      startProceduraGenrazioneMappa();
      pthread_join(tidGeneratoreMappa, NULL);
      turno++;
      timerCount = TIME_LIMIT_IN_SECONDS;
    }
  }
}

void configuraSocket(struct sockaddr_in mio_indirizzo)
{
  if ((socketDesc = socket(PF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("Impossibile creare socket");
    exit(-1);
  }
  if (setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <
      0)
    perror("Impossibile impostare il riutilizzo dell'indirizzo ip e della "
           "porta\n");
  if ((bind(socketDesc, (struct sockaddr *)&mio_indirizzo,
            sizeof(mio_indirizzo))) < 0)
  {
    perror("Impossibile effettuare bind");
    exit(-1);
  }
}

PlayerStats gestisciInput(char grigliaDiGioco[ROWS][COLUMNS],
                          char grigliaOstacoli[ROWS][COLUMNS], char input,
                          PlayerStats giocatore, Obstacles *listaOstacoli,
                          Point deployCoords[], Point packsCoords[],
                          char name[])
{
  if (giocatore == NULL)
  {
    return NULL;
  }
  if (input == 'w' || input == 'W')
  {
    giocatore = gestisciW(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  }
  else if (input == 's' || input == 'S')
  {
    giocatore = gestisciS(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  }
  else if (input == 'a' || input == 'A')
  {
    giocatore = gestisciA(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  }
  else if (input == 'd' || input == 'D')
  {
    giocatore = gestisciD(grigliaDiGioco, grigliaOstacoli, giocatore,
                          listaOstacoli, deployCoords, packsCoords);
  }
  else if (input == 'p' || input == 'P')
  {
    giocatore = gestisciP(grigliaDiGioco, giocatore, deployCoords, packsCoords);
  }
  else if (input == 'c' || input == 'C')
  {
    giocatore =
        gestisciC(grigliaDiGioco, giocatore, deployCoords, packsCoords, name);
  }

  // aggiorna la posizione dell'utente
  return giocatore;
}

PlayerStats gestisciC(char grigliaDiGioco[ROWS][COLUMNS], PlayerStats giocatore,
                      Point deployCoords[], Point packsCoords[], char name[])
{
  pthread_t tid;
  if (giocatore->hasApack == 0)
  {
    return giocatore;
  }
  else
  {
    if (isOnCorrectDeployPoint(giocatore, deployCoords))
    {
      Args args = (Args)malloc(sizeof(struct argsToSend));
      args->userName = (char *)calloc(MAX_BUF, 1);
      strcpy(args->userName, name);
      args->flag = 1;
      pthread_create(&tid, NULL, fileWriter, (void *)args);
      giocatore->score += 10;
      if (giocatore->score > scoreMassimo){
        pthread_mutex_lock(&ScoreMassimoMutex);
        scoreMassimo = giocatore->score;
        pthread_mutex_unlock(&ScoreMassimoMutex);
      }
      giocatore->deploy[0] = -1;
      giocatore->deploy[1] = -1;
      giocatore->hasApack = 0;
    }
    else
    {
      if (!isOnAPack(giocatore, packsCoords) &&
          !isOnADeployPoint(giocatore, deployCoords))
      {
        int index = getHiddenPack(packsCoords);
        if (index >= 0)
        {
          packsCoords[index]->x = giocatore->position[0];
          packsCoords[index]->y = giocatore->position[1];
          giocatore->hasApack = 0;
          giocatore->deploy[0] = -1;
          giocatore->deploy[1] = -1;
        }
      }
      else
        return giocatore;
    }
  }
  return giocatore;
}

void sendPlayerList(int clientDesc)
{
  int lunghezza = 0;
  char name[100];
  Players tmp = onLineUsers;
  int numeroClientLoggati = dimensioneLista(tmp);
  printf("%d ", numeroClientLoggati);
  if (!clientDisconnesso(clientDesc))
  {
    write(clientDesc, &numeroClientLoggati, sizeof(numeroClientLoggati));
    while (numeroClientLoggati > 0 && tmp != NULL)
    {
      strcpy(name, tmp->name);
      lunghezza = strlen(tmp->name);
      write(clientDesc, &lunghezza, sizeof(lunghezza));
      write(clientDesc, name, lunghezza);
      tmp = tmp->next;
      numeroClientLoggati--;
    }
  }
}

void prepareMessageForPackDelivery(char message[], char username[], char date[])
{
  strcat(message, "Pack delivered by \"");
  strcat(message, username);
  strcat(message, "\" at ");
  strcat(message, date);
  strcat(message, "\n");
}

void prepareMessageForLogin(char message[], char username[], char date[])
{
  strcat(message, username);
  strcat(message, "\" logged in at ");
  strcat(message, date);
  strcat(message, "\n");
}

void prepareMessageForConnection(char message[], char ipAddress[], char date[])
{
  strcat(message, ipAddress);
  strcat(message, "\" connected at ");
  strcat(message, date);
  strcat(message, "\n");
}

void putCurrentDateAndTimeInString(char dateAndTime[])
{
  time_t t = time(NULL);
  struct tm *infoTime = localtime(&t);
  strftime(dateAndTime, 64, "%X %x", infoTime);
}

void *fileWriter(void *args)
{
  int fDes = open("Log", O_RDWR | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);
  if (fDes < 0)
  {
    perror("Error while opening log file");
    exit(-1);
  }
  Args info = (Args)args;
  char dateAndTime[64];
  putCurrentDateAndTimeInString(dateAndTime);
  if (logDelPacco(info->flag))
  {
    char message[MAX_BUF] = "";
    prepareMessageForPackDelivery(message, info->userName, dateAndTime);
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  }
  else if (logDelLogin(info->flag))
  {
    char message[MAX_BUF] = "\"";
    prepareMessageForLogin(message, info->userName, dateAndTime);
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  }
  else if (logDellaConnessione(info->flag))
  {
    char message[MAX_BUF] = "\"";
    prepareMessageForConnection(message, info->userName, dateAndTime);
    pthread_mutex_lock(&LogMutex);
    write(fDes, message, strlen(message));
    pthread_mutex_unlock(&LogMutex);
  }
  close(fDes);
  free(info);
  pthread_exit(NULL);
}

void spostaPlayer(char griglia[ROWS][COLUMNS], int vecchiaPosizione[2],
                  int nuovaPosizione[2], Point deployCoords[],
                  Point packsCoords[])
{

  pthread_mutex_lock(&MatrixMutex);
  griglia[nuovaPosizione[0]][nuovaPosizione[1]] = 'P';
  if (eraUnPuntoDepo(vecchiaPosizione, deployCoords))
    griglia[vecchiaPosizione[0]][vecchiaPosizione[1]] = '_';
  else if (eraUnPacco(vecchiaPosizione, packsCoords))
    griglia[vecchiaPosizione[0]][vecchiaPosizione[1]] = '$';
  else
    griglia[vecchiaPosizione[0]][vecchiaPosizione[1]] = '-';
  pthread_mutex_unlock(&MatrixMutex);
}

PlayerStats gestisciW(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[])
{
  if (giocatore == NULL)
    return NULL;
  int nuovaPosizione[2];
  nuovaPosizione[1] = giocatore->position[1];
  // Aggiorna la posizione vecchia spostando il player avanti di 1
  nuovaPosizione[0] = (giocatore->position[0]) - 1;
  int nuovoScore = giocatore->score;
  int nuovoDeploy[2];
  nuovoDeploy[0] = giocatore->deploy[0];
  nuovoDeploy[1] = giocatore->deploy[1];
  if (nuovaPosizione[0] >= 0 && nuovaPosizione[0] < ROWS)
  {
    if (casellaVuotaOValida(grigliaDiGioco, grigliaOstacoli, nuovaPosizione))
    {
      spostaPlayer(grigliaDiGioco, giocatore->position, nuovaPosizione,
                   deployCoords, packsCoords);
    }
    else if (colpitoOstacolo(grigliaOstacoli, nuovaPosizione))
    {
      *listaOstacoli =
          addObstacle(*listaOstacoli, nuovaPosizione[0], nuovaPosizione[1]);
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    else if (colpitoPlayer(grigliaDiGioco, nuovaPosizione))
    {
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    giocatore->deploy[0] = nuovoDeploy[0];
    giocatore->deploy[1] = nuovoDeploy[1];
    giocatore->score = nuovoScore;
    giocatore->position[0] = nuovaPosizione[0];
    giocatore->position[1] = nuovaPosizione[1];
  }
  return giocatore;
}

PlayerStats gestisciD(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[])
{
  if (giocatore == NULL)
  {
    return NULL;
  }
  int nuovaPosizione[2];
  nuovaPosizione[1] = giocatore->position[1] + 1;
  nuovaPosizione[0] = giocatore->position[0];
  int nuovoScore = giocatore->score;
  int nuovoDeploy[2];
  nuovoDeploy[0] = giocatore->deploy[0];
  nuovoDeploy[1] = giocatore->deploy[1];
  if (nuovaPosizione[1] >= 0 && nuovaPosizione[1] < COLUMNS)
  {
    if (casellaVuotaOValida(grigliaDiGioco, grigliaOstacoli, nuovaPosizione))
    {
      spostaPlayer(grigliaDiGioco, giocatore->position, nuovaPosizione,
                   deployCoords, packsCoords);
    }
    else if (colpitoOstacolo(grigliaOstacoli, nuovaPosizione))
    {
      printf("Ostacolo\n");
      *listaOstacoli =
          addObstacle(*listaOstacoli, nuovaPosizione[0], nuovaPosizione[1]);
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    else if (colpitoPlayer(grigliaDiGioco, nuovaPosizione))
    {
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    giocatore->deploy[0] = nuovoDeploy[0];
    giocatore->deploy[1] = nuovoDeploy[1];
    giocatore->score = nuovoScore;
    giocatore->position[0] = nuovaPosizione[0];
    giocatore->position[1] = nuovaPosizione[1];
  }
  return giocatore;
}
PlayerStats gestisciA(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[])
{
  if (giocatore == NULL)
    return NULL;
  int nuovaPosizione[2];
  nuovaPosizione[0] = giocatore->position[0];
  // Aggiorna la posizione vecchia spostando il player avanti di 1
  nuovaPosizione[1] = (giocatore->position[1]) - 1;
  int nuovoScore = giocatore->score;
  int nuovoDeploy[2];
  nuovoDeploy[0] = giocatore->deploy[0];
  nuovoDeploy[1] = giocatore->deploy[1];
  if (nuovaPosizione[1] >= 0 && nuovaPosizione[1] < COLUMNS)
  {
    if (casellaVuotaOValida(grigliaDiGioco, grigliaOstacoli, nuovaPosizione))
    {
      printf("Casella vuota \n");
      spostaPlayer(grigliaDiGioco, giocatore->position, nuovaPosizione,
                   deployCoords, packsCoords);
    }
    else if (colpitoOstacolo(grigliaOstacoli, nuovaPosizione))
    {
      printf("Ostacolo\n");
      *listaOstacoli =
          addObstacle(*listaOstacoli, nuovaPosizione[0], nuovaPosizione[1]);
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    else if (colpitoPlayer(grigliaDiGioco, nuovaPosizione))
    {
      printf("colpito player\n");
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    giocatore->deploy[0] = nuovoDeploy[0];
    giocatore->deploy[1] = nuovoDeploy[1];
    giocatore->score = nuovoScore;
    giocatore->position[0] = nuovaPosizione[0];
    giocatore->position[1] = nuovaPosizione[1];
  }
  return giocatore;
}
PlayerStats gestisciS(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli,
                      Point deployCoords[], Point packsCoords[])
{
  if (giocatore == NULL)
  {
    return NULL;
  }
  // crea le nuove statistiche
  int nuovaPosizione[2];
  nuovaPosizione[1] = giocatore->position[1];
  nuovaPosizione[0] = (giocatore->position[0]) + 1;
  int nuovoScore = giocatore->score;
  int nuovoDeploy[2];
  nuovoDeploy[0] = giocatore->deploy[0];
  nuovoDeploy[1] = giocatore->deploy[1];
  // controlla che le nuove statistiche siano corrette
  if (nuovaPosizione[0] >= 0 && nuovaPosizione[0] < ROWS)
  {
    if (casellaVuotaOValida(grigliaDiGioco, grigliaOstacoli, nuovaPosizione))
    {
      spostaPlayer(grigliaDiGioco, giocatore->position, nuovaPosizione,
                   deployCoords, packsCoords);
    }
    else if (colpitoOstacolo(grigliaOstacoli, nuovaPosizione))
    {
      printf("Ostacolo\n");
      *listaOstacoli =
          addObstacle(*listaOstacoli, nuovaPosizione[0], nuovaPosizione[1]);
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    else if (colpitoPlayer(grigliaDiGioco, nuovaPosizione))
    {
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    giocatore->deploy[0] = nuovoDeploy[0];
    giocatore->deploy[1] = nuovoDeploy[1];
    giocatore->score = nuovoScore;
    giocatore->position[0] = nuovaPosizione[0];
    giocatore->position[1] = nuovaPosizione[1];
  }
  return giocatore;
}

int logDelPacco(int flag)
{
  if (flag == 1)
    return 1;
  return 0;
}
int logDelLogin(int flag)
{
  if (flag == 0)
    return 1;
  return 0;
}
int logDellaConnessione(int flag)
{
  if (flag == 2)
    return 1;
  return 0;
}
```

Codice sorgente boardUtility
----------------------------

``` {.c caption="Codice header utility del gioco 1"}
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define ROWS 10
#define COLUMNS 30
#define numberOfObstacles 35
#define numberOfPackages 15
#define TIME_LIMIT_IN_SECONDS 30
#define packageLimitNumber 4
#define MATRIX_DIMENSION sizeof(char) * ROWS *COLUMNS
#define RED_COLOR "\x1b[31m"
#define GREEN_COLOR "\x1b[32m"
#define RESET_COLOR "\x1b[0m"

struct Coord {
  int x;
  int y;
};
typedef struct Coord *Point;
void rimuoviPaccoDaArray(int posizione[2], Point packsCoords[]);
void printMenu();
int getHiddenPack(Point packsCoords[]);
int casellaVuotaOValida(char grigliaDiGioco[ROWS][COLUMNS],
                        char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]);
void stampaIstruzioni(int i);
int isOnADeployPoint(PlayerStats giocatore, Point deployCoords[]);
int isOnCorrectDeployPoint(PlayerStats giocatore, Point deployCoords[]);
PlayerStats gestisciP(char grigliaDiGioco[ROWS][COLUMNS], PlayerStats giocatore,
                      Point deployCoords[], Point packsCoords[]);
void inizializzaGiocoSenzaPlayer(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaConOstacoli[ROWS][COLUMNS],
                                 Point packsCoords[]);
void inserisciPlayerNellaGrigliaInPosizioneCasuale(
    char grigliaDiGioco[ROWS][COLUMNS], char grigliaOstacoli[ROWS][COLUMNS],
    int posizione[2]);
void inizializzaGrigliaVuota(char grigliaDiGioco[ROWS][COLUMNS]);
void generaPosizioneOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS]);
void riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
    char grigliaDiGioco[ROWS][COLUMNS], Point packsCoords[]);
void printGrid(char grigliaDaStampare[ROWS][COLUMNS], PlayerStats stats);
void start(char grigliaDiGioco[ROWS][COLUMNS],
           char grigliaOstacoli[ROWS][COLUMNS]);
void riempiGrigliaConGliOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaOstacoli[ROWS][COLUMNS]);
void generaPosizioniRaccolta(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS],
                             Point coord[]);
void mergeGridAndList(char grid[ROWS][COLUMNS], Obstacles top);
void scegliPosizioneRaccolta(Point coord[], int deploy[]);
int colpitoOstacolo(char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]);
int colpitoPacco(Point packsCoords[], int posizione[2]);
int colpitoPlayer(char grigliaDiGioco[ROWS][COLUMNS], int posizione[2]);
int casellaVuota(char grigliaDiGioco[ROWS][COLUMNS],
                 char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]);
int arrivatoADestinazione(int posizione[2], int destinazione[2]);
int eraUnPuntoDepo(int vecchiaPosizione[2], Point depo[]);
int eraUnPacco(int vecchiaPosizione[2], Point packsCoords[]);
int isOnAPack(PlayerStats giocatore, Point packsCoords[]);
```

``` {#alg:boardUtility .c caption="Codice sorgente utility del gioco 1" label="alg:boardUtility"}
#include "boardUtility.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
void printMenu() {
  system("clear");
  printf("\t Cosa vuoi fare?\n");
  printf("\t1 Gioca\n");
  printf("\t2 Registrati\n");
  printf("\t3 Esci\n");
}
int colpitoOstacolo(char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]) {
  if (grigliaOstacoli[posizione[0]][posizione[1]] == 'O')
    return 1;
  return 0;
}
int colpitoPacco(Point packsCoords[], int posizione[2]) {
  int i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    if (packsCoords[i]->x == posizione[0] && packsCoords[i]->y == posizione[1])
      return 1;
  }
  return 0;
}
int casellaVuotaOValida(char grigliaDiGioco[ROWS][COLUMNS],
                        char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]) {
  if (grigliaDiGioco[posizione[0]][posizione[1]] == '-' ||
      grigliaDiGioco[posizione[0]][posizione[1]] == '_' ||
      grigliaDiGioco[posizione[0]][posizione[1]] == '$')
    if (grigliaOstacoli[posizione[0]][posizione[1]] == '-' ||
        grigliaOstacoli[posizione[0]][posizione[1]] == '_' ||
        grigliaOstacoli[posizione[0]][posizione[1]] == '$')
      return 1;
  return 0;
}
int colpitoPlayer(char grigliaDiGioco[ROWS][COLUMNS], int posizione[2]) {
  if (grigliaDiGioco[posizione[0]][posizione[1]] == 'P')
    return 1;
  return 0;
}
int isOnCorrectDeployPoint(PlayerStats giocatore, Point deployCoords[]) {
  int i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    if (giocatore->deploy[0] == deployCoords[i]->x &&
        giocatore->deploy[1] == deployCoords[i]->y) {
      if (deployCoords[i]->x == giocatore->position[0] &&
          deployCoords[i]->y == giocatore->position[1])
        return 1;
    }
  }
  return 0;
}
int getHiddenPack(Point packsCoords[]) {
  int i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    if (packsCoords[i]->x == -1 && packsCoords[i]->y == -1)
      return i;
  }
  return -1;
}
int isOnAPack(PlayerStats giocatore, Point packsCoords[]) {
  int i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    if (giocatore->position[0] == packsCoords[i]->x &&
        giocatore->position[1] == packsCoords[i]->y)
      return 1;
  }
  return 0;
}
int isOnADeployPoint(PlayerStats giocatore, Point deployCoords[]) {
  int i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    if (giocatore->position[0] == deployCoords[i]->x &&
        giocatore->position[1] == deployCoords[i]->y)
      return 1;
  }
  return 0;
}
void inizializzaGrigliaVuota(char griglia[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      griglia[i][j] = '-';
    }
  }
}
PlayerStats gestisciP(char grigliaDiGioco[ROWS][COLUMNS], PlayerStats giocatore,
                      Point deployCoords[], Point packsCoords[]) {
  int nuovoDeploy[2];
  if (colpitoPacco(packsCoords, giocatore->position) &&
      giocatore->hasApack == 0) {
    scegliPosizioneRaccolta(deployCoords, nuovoDeploy);
    giocatore->hasApack = 1;
    rimuoviPaccoDaArray(giocatore->position, packsCoords);
  }
  giocatore->deploy[0] = nuovoDeploy[0];
  giocatore->deploy[1] = nuovoDeploy[1];
  return giocatore;
}

void printGrid(char grigliaDaStampare[ROWS][COLUMNS], PlayerStats stats) {
  system("clear");
  printf("\n\n");
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    printf("\t");
    for (j = 0; j < COLUMNS; j++) {
      if (stats != NULL) {
        if ((i == stats->deploy[0] && j == stats->deploy[1]) ||
            (i == stats->position[0] && j == stats->position[1]))
          if (grigliaDaStampare[i][j] == 'P' && stats->hasApack == 1)
            printf(GREEN_COLOR "%c" RESET_COLOR, grigliaDaStampare[i][j]);
          else
            printf(RED_COLOR "%c" RESET_COLOR, grigliaDaStampare[i][j]);
        else
          printf("%c", grigliaDaStampare[i][j]);
      } else
        printf("%c", grigliaDaStampare[i][j]);
    }
    stampaIstruzioni(i);
    if (i == 8)
      printf(GREEN_COLOR "\t\t Punteggio: %d" RESET_COLOR, stats->score);
    printf("\n");
  }
}
void stampaIstruzioni(int i) {
  if (i == 0)
    printf("\t \t ISTRUZIONI ");
  if (i == 1)
    printf("\t Inviare 't' per il timer.");
  if (i == 2)
    printf("\t Inviare 'e' per uscire");
  if (i == 3)
    printf("\t Inviare 'p' per raccogliere un pacco");
  if (i == 4)
    printf("\t Inviare 'c' per consegnare il pacco");
  if (i == 5)
    printf("\t Inviare 'w'/'s' per andare sopra/sotto");
  if (i == 6)
    printf("\t Inviare 'a'/'d' per andare a dx/sx");
  if (i == 7)
    printf("\t Inviare 'l' per la lista degli utenti ");
}
// aggiunge alla griglia gli ostacoli visti fino ad ora dal client
void mergeGridAndList(char grid[ROWS][COLUMNS], Obstacles top) {
  while (top) {
    grid[top->x][top->y] = 'O';
    top = top->next;
  }
}
/* Genera la posizione degli ostacoli */
void generaPosizioneOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS]) {
  int x, y, i;
  inizializzaGrigliaVuota(grigliaOstacoli);
  srand(time(0));
  for (i = 0; i < numberOfObstacles; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grigliaDiGioco[y][x] == '-')
      grigliaOstacoli[y][x] = 'O';
    else
      i--;
  }
}
void rimuoviPaccoDaArray(int posizione[2], Point packsCoords[]) {
  int i = 0, found = 0;
  while (i < numberOfPackages && !found) {
    if ((packsCoords[i])->x == posizione[0] &&
        (packsCoords[i])->y == posizione[1]) {
      (packsCoords[i])->x = -1;
      (packsCoords[i])->y = -1;
      found = 1;
    }
    i++;
  }
}
// sceglie una posizione di raccolta tra quelle disponibili
void scegliPosizioneRaccolta(Point coord[], int deploy[]) {
  int index = 0;
  srand(time(NULL));
  index = rand() % numberOfPackages;
  deploy[0] = coord[index]->x;
  deploy[1] = coord[index]->y;
}
/*genera posizione di raccolta di un pacco*/
void generaPosizioniRaccolta(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS],
                             Point coord[]) {
  int x, y;
  srand(time(0));
  int i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    coord[i] = (Point)malloc(sizeof(struct Coord));
  }
  i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grigliaDiGioco[y][x] == '-' && grigliaOstacoli[y][x] == '-') {
      coord[i]->x = y;
      coord[i]->y = x;
      grigliaDiGioco[y][x] = '_';
      grigliaOstacoli[y][x] = '_';
    } else
      i--;
  }
}
/*Inserisci dei pacchi nella griglia di gioco nella posizione casuale */
void riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
    char grigliaDiGioco[ROWS][COLUMNS], Point packsCoords[]) {
  int x, y, i = 0;
  for (i = 0; i < numberOfPackages; i++) {
    packsCoords[i] = (Point)malloc(sizeof(struct Coord));
  }
  srand(time(0));
  for (i = 0; i < numberOfPackages; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grigliaDiGioco[y][x] == '-') {
      grigliaDiGioco[y][x] = '$';
      packsCoords[i]->x = y;
      packsCoords[i]->y = x;
    } else
      i--;
  }
}
/*Inserisci gli ostacoli nella griglia di gioco */
void riempiGrigliaConGliOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaOstacoli[ROWS][COLUMNS]) {
  int i, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      if (grigliaOstacoli[i][j] == 'O')
        grigliaDiGioco[i][j] = 'O';
    }
  }
}
void inserisciPlayerNellaGrigliaInPosizioneCasuale(
    char grigliaDiGioco[ROWS][COLUMNS], char grigliaOstacoli[ROWS][COLUMNS],
    int posizione[2]) {
  int x, y;
  srand(time(0));
  printf("Inserisco player\n");
  do {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
  } while (grigliaDiGioco[y][x] != '-' && grigliaOstacoli[y][x] != '-');
  grigliaDiGioco[y][x] = 'P';
  posizione[0] = y;
  posizione[1] = x;
}
void inizializzaGiocoSenzaPlayer(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaConOstacoli[ROWS][COLUMNS],
                                 Point packsCoords[]) {
  inizializzaGrigliaVuota(grigliaDiGioco);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(grigliaDiGioco,
                                                       packsCoords);
  generaPosizioneOstacoli(grigliaDiGioco, grigliaConOstacoli);
  return;
}

int eraUnPuntoDepo(int vecchiaPosizione[2], Point depo[]) {
  int i = 0, ret = 0;
  while (ret == 0 && i < numberOfPackages) {
    if ((depo[i])->y == vecchiaPosizione[1] &&
        (depo[i])->x == vecchiaPosizione[0]) {
      ret = 1;
    }
    i++;
  }
  return ret;
}
int eraUnPacco(int vecchiaPosizione[2], Point packsCoords[]) {
  int i = 0, ret = 0;
  while (ret == 0 && i < numberOfPackages) {
    if ((packsCoords[i])->y == vecchiaPosizione[1] &&
        (packsCoords[i])->x == vecchiaPosizione[0]) {
      ret = 1;
    }
    i++;
  }
  return ret;
}

int arrivatoADestinazione(int posizione[2], int destinazione[2]) {
  if (posizione[0] == destinazione[0] && posizione[1] == destinazione[1])
    return 1;
  return 0;
}
```

Codice sorgente list
--------------------

``` {.c caption="Codice header utility del gioco 2"}
#ifndef DEF_LIST_H
#define DEF_LIST_H
#define MAX_BUF 200
#include <pthread.h>
// players
struct TList {
  char *name;
  struct TList *next;
  int sockDes;
} TList;

struct Data {
  int deploy[2];
  int score;
  int position[2];
  int hasApack;
} Data;

// Obstacles
struct TList2 {
  int x;
  int y;
  struct TList2 *next;
} TList2;

typedef struct Data *PlayerStats;
typedef struct TList *Players;
typedef struct TList2 *Obstacles;

// calcola e restituisce il numero di player commessi dalla lista L
int dimensioneLista(Players L);

// inizializza un giocatore
Players initPlayerNode(char *name, int sockDes);

// Crea un nodo di Stats da mandare a un client
PlayerStats initStats(int deploy[], int score, int position[], int flag);

// Inizializza un nuovo nodo
Players initNodeList(char *name, int sockDes);

// Aggiunge un nodo in testa alla lista
// La funzione ritorna sempre la testa della lista
Players addPlayer(Players L, char *name, int sockDes);

// Rimuove solo un occorrenza di un nodo con il socket Descriptor
//  specificato dalla lista
// La funzione ritorna sempre la testa della lista
Players removePlayer(Players L, int sockDes);

// Dealloca la lista interamente
void freePlayers(Players L);

// Stampa la lista
void printPlayers(Players L);

// Controlla se un utente è già loggato
int isAlreadyLogged(Players L, char *name);

// Dealloca la lista degli ostacoli
void freeObstacles(Obstacles L);

// Stampa la lista degli ostacoli
void printObstacles(Obstacles L);

// Aggiunge un ostacolo in testa
Obstacles addObstacle(Obstacles L, int x, int y);

// Inizializza un nuovo nodo ostacolo
Obstacles initObstacleNode(int x, int y);
#endif
```

``` {#alg:list .c caption="Codice sorgente utility del gioco 2" label="alg:list"}
#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Players initPlayerNode(char *name, int sockDes) {
  Players L = (Players)malloc(sizeof(struct TList));
  L->name = (char *)malloc(MAX_BUF);
  strcpy(L->name, name);
  L->sockDes = sockDes;
  L->next = NULL;
  return L;
}
PlayerStats initStats(int deploy[], int score, int position[], int flag) {
  PlayerStats L = (PlayerStats)malloc(sizeof(struct Data));
  L->deploy[0] = deploy[0];
  L->deploy[1] = deploy[1];
  L->score = score;
  L->hasApack = flag;
  L->position[0] = position[0];
  L->position[1] = position[1];
  return L;
}
Obstacles initObstacleNode(int x, int y) {
  Obstacles L = (Obstacles)malloc(sizeof(struct TList2));
  L->x = x;
  L->y = y;
  L->next = NULL;
  return L;
}
Obstacles addObstacle(Obstacles L, int x, int y) {
  Obstacles tmp = initObstacleNode(x, y);
  if (L != NULL)
    tmp->next = L;
  return tmp;
}
int dimensioneLista(Players L) {
  int size = 0;
  Players tmp = L;
  while (tmp != NULL) {
    size++;
    tmp = tmp->next;
  }
  return size;
}
int isAlreadyLogged(Players L, char *name) {
  int ret = 0;
  if (L != NULL) {
    if (strcmp(L->name, name) == 0)
      return 1;
    ret = isAlreadyLogged(L->next, name);
  }
  return ret;
}
Players addPlayer(Players L, char *name, int sockDes) {
  Players tmp = initPlayerNode(name, sockDes);
  if (L != NULL)
    tmp->next = L;
  return tmp;
}
Players removePlayer(Players L, int sockDes) {
  if (L != NULL) {
    if (L->sockDes == sockDes) {
      Players tmp = L->next;
      free(L);
      return tmp;
    }
    L->next = removePlayer(L->next, sockDes);
  }
  return L;
}
void freePlayers(Players L) {
  if (L != NULL) {
    freePlayers(L->next);
    free(L);
  }
}
void freeObstacles(Obstacles L) {
  if (L != NULL) {
    freeObstacles(L->next);
    free(L);
  }
}
void printPlayers(Players L) {
  if (L != NULL) {
    printf("%s ->", L->name);
    printPlayers(L->next);
  }
  printf("\n");
}
void printObstacles(Obstacles L) {
  if (L != NULL) {
    printf("X:%d Y:%d ->", L->x, L->y);
    printObstacles(L->next);
  }
}
```

Codice sorgente parser
----------------------

``` {.c caption="Codice header utility del gioco 3"}
int appendPlayer(char *name, char *pwd, char *file);
int isRegistered(char *name, char *file);
int openFileRDWRAPP(char *file);
int validateLogin(char *name, char *pwd, char *file);
int openFileRDON(char *file);
void premiEnterPerContinuare();
```

``` {#alg:parser .c caption="Codice sorgente utility del gioco 3" label="alg:parser"}
#include "parser.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_BUF 200
int openFileRDWRAPP(char *file) {
  int fileDes = open(file, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (fileDes < 0)
    perror("Errore apertura file\n"), exit(-1);
  return fileDes;
}
int openFileRDON(char *file) {
  int fileDes = open(file, O_RDONLY);
  if (fileDes < 0)
    perror("Errore apertura file\n"), exit(-1);
  return fileDes;
}
int appendPlayer(char *name, char *pwd, char *file) {
  if (isRegistered(name, file))
    return 0;
  int fileDes = openFileRDWRAPP(file);
  write(fileDes, name, strlen(name));
  write(fileDes, " ", 1);
  write(fileDes, pwd, strlen(pwd));
  write(fileDes, "\n", 1);
  close(fileDes);
  return 1;
}
int isRegistered(char *name, char *file) {
  char command[MAX_BUF] = "cat  ";
  strcat(command, file);
  char toApp[] = " |cut -d\" \" -f1|grep \"^";
  strcat(command, toApp);
  strcat(command, name);
  char toApp2[] = "$\">tmp";
  strcat(command, toApp2);
  int ret = 0;
  system(command);
  int fileDes = openFileRDON("tmp");
  struct stat info;
  fstat(fileDes, &info);
  if ((int)info.st_size > 0)
    ret = 1;
  close(fileDes);
  system("rm tmp");
  return ret;
}
int validateLogin(char *name, char *pwd, char *file) {
  if (!isRegistered(name, file))
    return 0;
  char command[MAX_BUF] = "cat  ";
  strcat(command, file);
  char toApp[] = " |grep \"^";
  strcat(command, toApp);
  strcat(command, name);
  strcat(command, " ");
  strcat(command, pwd);
  char toApp2[] = "$\">tmp";
  strcat(command, toApp2);
  int ret = 0;
  system(command);
  int fileDes = openFileRDON("tmp");
  struct stat info;
  fstat(fileDes, &info);
  if ((int)info.st_size > 0)
    ret = 1;
  close(fileDes);
  system("rm tmp");
  return ret;
}
void premiEnterPerContinuare() {
  fflush(stdin);
  printf("Premi Invio per continuare\n");
  char c = getchar();
}
```

[^1]: Altro dettaglio meno importante, ma comunque degno di nota è il
    fatto che il timer non inizia il conto alla rovescia se non c'è
    almeno un giocatore loggato, se questo non è stato posizionato sulla
    mappa e se questo non ha effettuato la prima mossa. Al rigo 3 c'è
    anche da giustificare la variabile \"cambiato\" che non è
    nient'altro che un flag, il quale impedisce al server di stampare in
    stdout il valore del timer nel caso in cui esso sia stato appena
    resettato e non sia ancora iniziato il conto alla rovescia. Ciò
    evita che, prima che inizi il conto alla rovescia, il server
    continui a stampare il valore massimo del timer
