#include "boardUtility.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>



/*int main(int argc, char *argv[]) {
      char grigliaDiGioco[ROWS][COLUMNS];
      char grigliaOstacoli[ROWS][COLUMNS];

      start(grigliaDiGioco, grigliaOstacoli);
      return 0;
    }*/
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
int colpitoPacco(char grigliaDiGioco[ROWS][COLUMNS], int posizione[2]) {
  if (grigliaDiGioco[posizione[0]][posizione[1]] == '$')
    return 1;
  return 0;
}
int casellaVuota(char grigliaDiGioco[ROWS][COLUMNS],
                 char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]) {
  if (grigliaDiGioco[posizione[0]][posizione[1]] == '-')
    if (grigliaOstacoli[posizione[0]][posizione[1]] == '-')
      return 1;
  return 0;
}
int colpitoPlayer(char grigliaDiGioco[ROWS][COLUMNS], int posizione[2]) {
  if (grigliaDiGioco[posizione[0]][posizione[1]] == 'P')
    return 1;
  return 0;
}

PlayerStats gestisciInput(char grigliaDiGioco[ROWS][COLUMNS],
                          char grigliaOstacoli[ROWS][COLUMNS], char input,
                          PlayerStats giocatore, Obstacles listaOstacoli) {
  PlayerStats nuoveStatistiche;
  if (input == 'w') {
    nuoveStatistiche =
        gestisciW(grigliaDiGioco, grigliaOstacoli, giocatore, listaOstacoli);
  } /*else if (input == 's') {
    if (riga - 1 > 0) {
      grigliaDiGioco[riga][colonna] = '-';
      riga--;
      grigliaDiGioco[riga][colonna] = 'P';
    }
  } else if (input == 'a') {
    if (colonna - 1 > 0) {
      grigliaDiGioco[riga][colonna] = '-';
      colonna--;
      grigliaDiGioco[riga][colonna] = 'P';
    }
  } else if (input == 'd') {
    if (colonna + 1 < COLUMNS) {
      grigliaDiGioco[riga][colonna] = '-';
      colonna++;
      grigliaDiGioco[riga][colonna] = 'P';
    }
  }*/

  // aggiorna la posizione dell'utente
  return nuoveStatistiche;
}

// TODO da cancellare
/*
void start(char grigliaDiGioco[ROWS][COLUMNS],
           char grigliaOstacoli[ROWS][COLUMNS]) {
  int movement;
  int riga = 0, colonna = 0;
  inizializzaGrigliaVuota(grigliaDiGioco);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(grigliaDiGioco);
  generaPosizioneOstacoli(grigliaDiGioco, grigliaOstacoli);
  system("clear");
  // riempiGrigliaConGliOstacoli(grigliaDiGioco, grigliaOstacoli);
  printGrid(grigliaDiGioco);
  while (1) {
    movement = getchar();
    if (movement == EOF) {
      break;
    }
    switch (movement) {
    case 'w':
      if ((riga - 1) >= 0) {
        grigliaDiGioco[riga][colonna] = '-';
        riga = riga - 1;
      }
      break;
    case 's':
      if ((riga + 1) < ROWS) {
        grigliaDiGioco[riga][colonna] = '-';
        riga = riga + 1;
      }
      break;
    case 'a':
      if ((colonna - 1) >= 0) {
        grigliaDiGioco[riga][colonna] = '-';
        colonna = (colonna - 1) % COLUMNS;
      }
      break;
    case 'd':
      if ((colonna + 1) < COLUMNS) {
        grigliaDiGioco[riga][colonna] = '-';
        colonna = (colonna + 1) % COLUMNS;
      }
      break;
    default:
      break;
    }
    if (movement == 'p') {
      gameOver();
      break;
    }
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
    if (ch == EOF) {
      break;
    }
    // system("clear");
    printGrid(grigliaDiGioco);
  }
}
*/
/*Svuota la griglia di gioco e la riempe solo di '-' */
void inizializzaGrigliaVuota(char griglia[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      griglia[i][j] = '-';
    }
  }
}
/* stampa a schermo la griglia passata in input */
void printGrid(char grigliaDaStampare[ROWS][COLUMNS], PlayerStats stats) {
  system("clear");
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    printf("\t");
    for (j = 0; j < COLUMNS; j++) {
      if (i == stats->deploy[0] && j == stats->deploy[1])
        printf(RED_COLOR "%c" RESET_COLOR, grigliaDaStampare[i][j]);
      else
        printf("%c", grigliaDaStampare[i][j]);
    }
    printf("\n");
  }
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
    if (grigliaDiGioco[y][x] == '-') {
      grigliaOstacoli[y][x] = 'O';
    } else
      i--;
  }
}

/*genera posizione di raccolta di un pacco*/
void generaPosizioneRaccolta(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS], int *coord,
                             int xPlayer, int yPlayer) {
  int x, y, done = 0;
  srand(time(0));
  while (!done) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grigliaDiGioco[y][x] == '-' && grigliaOstacoli[y][x] == '-' &&
        (y != xPlayer || yPlayer != x)) {
      coord[0] = y;
      coord[1] = x;
      done = 1;
    }
  }
}

/*Inserisci dei pacchi nella griglia di gioco nella posizione casuale */
void riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
    char grigliaDiGioco[ROWS][COLUMNS]) {
  int x, y, i;
  srand(time(0));
  for (i = 0; i < numberOfPackages; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grigliaDiGioco[y][x] == '-') {
      grigliaDiGioco[y][x] = '$';
    } else
      i--;
  }
}
/*Stampa a schermo la griglia degli ostacoli */
void printObs(char grigliaOstacoli[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < 50; i++) {
    printf("(");
    for (j = 0; j < 2; j++) {
      printf("%d,", grigliaOstacoli[i][j]);
    }
    printf("); ");
  }
  printf("\n");
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
/*Inserisci il player in una posizione casuale*/
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

/*Genera sia la griglia con i pacchi che la griglia con gli ostacoli*/
void inizializzaGiocoSenzaPlayer(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaConOstacoli[ROWS][COLUMNS]) {

  inizializzaGrigliaVuota(grigliaDiGioco);
  riempiGrigliaConPacchiInPosizioniGenerateCasualmente(grigliaDiGioco);
  generaPosizioneOstacoli(grigliaDiGioco, grigliaConOstacoli);
  return;
}

PlayerStats gestisciW(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles listaOstacoli) {
  PlayerStats nuoveStatistiche = NULL;
  int nuovaPosizione[2];
  nuovaPosizione[1] = giocatore->position[1];

  // Aggiorna la posizione vecchia spostando il player avanti di 1
  nuovaPosizione[0] = (giocatore->position[0]) - 1;
  int nuovoScore = giocatore->score;
  int nuovoDeploy[2] = giocatore->deploy;

  int riga = giocatore->position[0];
  int colonna = giocatore->position[1];
  int nuovaRiga = riga;
  int nuovaColonna = colonna;
  if (nuovaRiga - 1 > 0) {
    nuovaRiga -= 1;
    if (casellaVuota(grigliaDiGioco, grigliaOstacoli, nuovaPosizione)) {
      aggiornaGrigliaW(grigliaDiGioco, giocatore->position, nuovaPosizione);
    } else if (colpitoPacco(grigliaDiGioco, giocatore->position)) {
      generaPosizioneRaccolta(grigliaDiGioco, grigliaOstacoli,
                              giocatore->deploy, giocatore->position[0],
                              giocatore->position[1]);
      aggiornaGrigliaW(grigliaDiGioco, giocatore->position, nuovaPosizione);
    } else if (arrivatoADestinazione(nuovaPosizione, giocatore->deploy)) {
      aggiornaGrigliaW(grigliaDiGioco, giocatore->position, nuovaPosizione);
      nuovoScore += 10;
      nuovoDeploy[0] = -1;
      nuovoDeploy[1] = -1;
    } else if (colpitoOstacolo(grigliaOstacoli, giocatore->position)) {
      *listaOstacoli =
          addObstacle(listaOstacoli, nuovaPosizione[0], nuovaPosizione[1]);
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    } else if (colpitoPlayer(grigliaDiGioco, giocatore->position)) {
      nuovaPosizione[0] = giocatore->position[0];
      nuovaPosizione[1] = giocatore->position[1];
    }
    nuoveStatistiche = initStats(nuovoDeploy, nuovoScore, nuovaPosizione);
    return nuoveStatistiche;
  }
}
void aggiornaGrigliaW(char griglia[ROWS][COLUMNS], int vecchiaPosizione[2],
                      int nuovaPosizione[2]) {
  griglia[vecchiaPosizione[0]][vecchiaPosizione[1]] = '-';
  griglia[nuovaPosizione[0]][nuovaPosizione[1]] = 'P';
}
int arrivatoADestinazione(int posizione[2], int destinazione[2]) {
  if (posizione[0] == destinazione[0] && posizione[1] == destinazione[1])
    return 1;
  return 0;
}