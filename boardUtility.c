#include "boardUtility.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define ROWS 20
#define COLUMNS 60
#define numberOfObstacles 50
#define numberOfPackages 30
#define TIME_LIMIT_IN_SECONDS 10
#define packageLimitNumber 8
#define MATRIX_DIMENSION sizeof(char) * ROWS *COLUMNS

void printMenu();
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
int gestisciW(char grigliaDiGioco[ROWS][COLUMNS],
              char grigliaOstacoli[ROWS][COLUMNS], int posizioneUtente[2],
              int destinazione[2], char input, int *punteggio) {
  int riga = posizioneUtente[0];
  int colonna = posizioneUtente[1];
  int nuovaRiga = riga;
  int nuovaColonna = colonna;
  if (nuovaRiga - 1 > 0) {
    nuovaRiga -= 1;
    // la casella è vuota
    if (grigliaDiGioco[nuovaRiga][colonna] == '-' &&
        grigliaOstacoli[nuovaRiga][nuovaColonna] == '-') {
      grigliaDiGioco[riga][colonna] = '-';
      grigliaDiGioco[nuovaRiga][colonna] = 'P';
      riga = nuovaRiga;
    } else if (grigliaDiGioco[nuovaRiga][colonna] == '$') {
      generaPosizioneRaccolta(grigliaDiGioco, grigliaOstacoli, destinazione,
                              posizioneUtente[0], posizioneUtente[1]);
      grigliaDiGioco[riga][colonna] = '-';
      grigliaDiGioco[nuovaRiga][colonna] = 'P';
    } else if (nuovaRiga == destinazione[0] &&
               nuovaColonna == destinazione[1]) {
      grigliaDiGioco[riga][colonna] = '-';
      grigliaDiGioco[nuovaRiga][colonna] = 'P';
      punteggio += 10;
    } else {
      fprintf(stdout, "Incontrato ostacolo o player\n");
      nuovaRiga += 1;
    }
    posizioneUtente[0] = riga;

    return 1;
  }
}
void gestisciInput(char grigliaDiGioco[ROWS][COLUMNS],
                   char grigliaOstacoli[ROWS][COLUMNS], int posizioneUtente[2],
                   int destinazione[2], char input, int *punteggio) {
  int riga = posizioneUtente[0];
  int colonna = posizioneUtente[1];
  int nuovaRiga = riga;
  int nuovaColonna = colonna;
  if (input == 'w') {
    gestisciW(grigliaDiGioco, grigliaOstacoli, posizioneUtente, destinazione,
              input, punteggio);
  } else if (input == 's') {
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
  }

  // aggiorna la posizione dell'utente
  posizioneUtente[0] = nuovaRiga;
  posizioneUtente[1] = nuovaColonna;
  return;
}
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
void printGrid(char grigliaDaStampare[ROWS][COLUMNS]) {
  system("clear");
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    printf("\t");
    for (j = 0; j < COLUMNS; j++) {
      printf("%c", grigliaDaStampare[i][j]);
    }
    printf("\n");
  }
}

//aggiunge alla griglia gli ostacoli visti fino ad ora dal client
void mergeGridAndList(char grid[ROWS][COLUMNS], Obstacles top){
  while(top){
    grid[top->x][top->y]='O';
    top=top->next;
  }
}

/* Stampa schermata di fine gioco */
void gameOver() {
  char c;

  printf("____________Game over_______________\n");
  fflush(stdin);
  scanf("%c", &c);
  return;
}

// TODO: bisogna aggiungere una matrice che mantiene la posizione degli
// ostacoli

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