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
  if (grigliaDiGioco[posizione[0]][posizione[1]] == '-' || // casella vuota
      grigliaDiGioco[posizione[0]][posizione[1]] == '_' || // punto deploy
      grigliaDiGioco[posizione[0]][posizione[1]] == '$')   // pacco
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
