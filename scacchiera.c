#include "scacchiera.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// void generatePlayerPosition(char);
// void printScore();
void fillGridInitializer();
void generateObstalePositions();
void generatePackagesPositions();
void printGrid(char grigliaDaStampare[ROWS][COLUMNS]);
void start();
void printAll();
void gameOver();
void printMenu();
void printObs();
char grid[ROWS][COLUMNS];
char obstaclesGrid[50][2];

void printObs() {
  int i = 0, j = 0;
  for (i = 0; i < 50; i++) {
    printf("(");
    for (j = 0; j < 2; j++) {
      printf("%d,", obstaclesGrid[i][j]);
    }
    printf("); ");
  }
  printf("\n");
}

int main(int argc, char *argv[]) {
  start();
  return 0;
}
void start() {
  int movement;
  int riga = 0, colonna = 0;
  fillGridInitializer();
  generateObstalePositions();
  generatePackagesPositions();
  // generatePlayerPosition("P");
  // grid[riga][colonna] = '#';
  system("clear");
  printAll();
  while (1) {
    movement = getchar();
    if (movement == EOF) {
      break;
    }
    switch (movement) {
    case 'w':
      if ((riga - 1) >= 0) {
        grid[riga][colonna] = '-';
        riga = riga - 1;
      }
      break;
    case 's':
      if ((riga + 1) < ROWS) {
        grid[riga][colonna] = '-';
        riga = riga + 1;
      }
      break;
    case 'a':
      if ((colonna - 1) >= 0) {
        grid[riga][colonna] = '-';
        colonna = (colonna - 1) % COLUMNS;
      }
      break;
    case 'd':
      if ((colonna + 1) < COLUMNS) {
        grid[riga][colonna] = '-';
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
    // grid[riga][colonna] = '#';
    printAll();
  }
}

void printAll() {
  // printScore();
  printGrid(grid);
  printObs();
}
void fillGridInitializer() {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      grid[i][j] = '-';
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
  // printf("\tYour score was: %d\n", score);
  // printf("\tPress enter to continue...\n");
  printf("____________Game over_______________\n");
  fflush(stdin);
  scanf("%c", &c);
  return;
}
void printMenu() {
  printf("\t Cosa vuoi fare?\n");
  printf("\t 1) Login\n");
  printf("\t 2) Registrati\n");
  printf("\t 3) Esci\n");
}

// TODO: bisogna aggiungere una matrice che mantiene la posizione degli
// ostacoli
void generateObstalePositions() {
  int indexOstacoli = 0;
  int x, y, i;
  srand(time(NULL));
  for (i = 0; i < numberOfObstacles; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grid[y][x] == '-') {
      grid[y][x] = '@';
      obstaclesGrid[indexOstacoli][0] = y;
      obstaclesGrid[indexOstacoli][1] = x;
      indexOstacoli++;
    } else
      i--;
  }
}

void generatePackagesPositions() {
  int x, y, i;
  srand(time(NULL));
  for (i = 0; i < numberOfPackages; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grid[y][x] == '-') {
      grid[y][x] = '$';
    } else
      i--;
  }
}
/*
void generatePlayerPosition(char n) {
  int x, y, i;
  char player;
  srand(time(NULL));
  for (i = 0; i < 1; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grid[y][x] == '-') {
      grid[y][x] = n;
    } else
      i--;
  }
}*/
/*
Questa funziona va rivista perchè ogni utente avrà un suo score
void printScore() { printf("\t SCORE: %d\n", score); }*/