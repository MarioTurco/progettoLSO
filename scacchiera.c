#include "scacchiera.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ROWS 20
#define COLUMNS 65
#define numberOfObstacles 50
/*
player prototype to be implemented in the future
struct player {
  char username[20];
  char password[20];
  int row;
  int column;
  int score;
} player;
*/
void fillGridInitializer();
void generateObstalePositions();
void printGrid();
void start();
void printScore();
void printAll();
void gameOver();
void printMenu();
char grid[ROWS][COLUMNS];
int score = 0;
char obstaclesGrid[50][2];

int main(int argc, char *argv[]) {
  start();
  return 0;
}
void start() {
  int movement;
  int riga = 0, colonna = 0;
  fillGridInitializer();
  generateObstalePositions();
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
  printScore();
  printGrid();
}
void fillGridInitializer() {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      grid[i][j] = '-';
    }
  }
}
void printScore() { printf("\t SCORE: %d\n", score); }
void printGrid() {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    printf("\t");
    for (j = 0; j < COLUMNS; j++) {
      printf("%c", grid[i][j]);
    }
    printf("\n");
  }
}
void gameOver() {
  char c;
  system("clear");
  printf("____________Game over_______________\n");
  printf("\tYour score was: %d\n", score);
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

void generateObstalePositions() {

  int x, y, i;
  int indiceRigaOstacoli = 0;
  srand(time(NULL));
  for (i = 0; i < numberOfObstacles; i++) {
    x = rand() % COLUMNS;
    y = rand() % ROWS;
    if (grid[y][x] == '-') {
      grid[y][x] = '@';
      // obstaclesGrid[indiceRigaOstacoli][0] = y;
      // obstaclesGrid[indiceRigaOstacoli][1] = x;
      //  printf("Generati: (%d, %d)\n", x, y);
    } else
      i--;
  }
}
