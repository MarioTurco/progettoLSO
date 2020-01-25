#include "boardUtility.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void inizializzaGrigliaVuota(char grigliaDiGioco[ROWS][COLUMNS]);
void generaPosizioneOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS]);
void generatePackagesPositions(char grigliaDiGioco[ROWS][COLUMNS],
                               char grigliaPacchi[numberOfPackages][2]);
void printGrid(char grigliaDaStampare[ROWS][COLUMNS]);
void start(char grigliaDiGioco[ROWS][COLUMNS],
           char grigliaOstacoli[ROWS][COLUMNS],
           char grigliaPacchi[numberOfPackages][2]);
void gameOver();
void printObs(char grigliaOstacoli[numberOfObstacles][2]);
void riempiGrigliaConGliOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaOstacoli[ROWS][COLUMNS]);
int main(int argc, char *argv[]) {
  char grigliaDiGioco[ROWS][COLUMNS];
  char grigliaOstacoli[numberOfObstacles][2];
  char grigliaPacchi[numberOfPackages][2];
  start(grigliaDiGioco, grigliaOstacoli, grigliaPacchi);
  return 0;
}
void start(char grigliaDiGioco[ROWS][COLUMNS],
           char grigliaOstacoli[ROWS][COLUMNS],
           char grigliaPacchi[numberOfPackages][2]) {
  int movement;
  int riga = 0, colonna = 0;
  inizializzaGrigliaVuota(grigliaDiGioco);
  generatePackagesPositions(grigliaDiGioco, grigliaPacchi);
  riempiGrigliaConIPacchi(grigliaDiGioco, grigliaPacchi);
  generaPosizioneOstacoli(grigliaDiGioco, grigliaOstacoli);
  system("clear");
  riempiGrigliaConGliOstacoli(grigliaDiGioco, grigliaOstacoli);
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
    system("clear");
    // grigliaDiGioco[riga][colonna] = '#';
    printGrid(grigliaDiGioco);
  }
}

void inizializzaGrigliaVuota(char griglia[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      griglia[i][j] = '-';
    }
  }
}
void printGrid(char grigliaDaStampare[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    printf("\t");
    for (j = 0; j < COLUMNS; j++) {
      printf("%c", grigliaDaStampare[i][j]);
    }
    printf("\n");
  }
}
void gameOver() {
  char c;
  system("clear");
  printf("____________Game over_______________\n");
  fflush(stdin);
  scanf("%c", &c);
  return;
}

// TODO: bisogna aggiungere una matrice che mantiene la posizione degli
// ostacoli
void generaPosizioneOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[numberOfObstacles][2]) {
  int indexOstacoli = 0;
  int x, y, i;
  srand(time(NULL));
  for (i = 0; i < numberOfObstacles; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grigliaDiGioco[y][x] == '-') {
      // grid[y][x] = '@';
      grigliaOstacoli[indexOstacoli][0] = y;
      grigliaOstacoli[indexOstacoli][1] = x;
      indexOstacoli++;
    } else
      i--;
  }
}

void generatePackagesPositions(char grigliaDiGioco[ROWS][COLUMNS],
                               char grigliaPacchi[numberOfPackages][2]) {
  int x, y, i;
  srand(time(NULL));
  for (i = 0; i < numberOfPackages; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grigliaDiGioco[y][x] == '-') {
      grigliaDiGioco[y][x] = '$';
    } else
      i--;
  }
}
void printObs(char grigliaOstacoli[numberOfObstacles][2]) {
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

void riempiGrigliaConGliOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaOstacoli[numberOfObstacles][2]) {
  int i, j = 0;
  int riga, colonna;
  for (i = 0; i < numberOfObstacles; i++) {
    riga = grigliaOstacoli[i][0];
    colonna = grigliaOstacoli[i][1];
    grigliaDiGioco[riga][colonna] = 'O';
  }
}