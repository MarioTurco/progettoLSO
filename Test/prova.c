#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define ROWS 3
#define COLUMNS 4

void printGrid(char grid[][COLUMNS]);
void update(int, int, char grid[][COLUMNS]);
void fillGrid(int, int, char grid[][COLUMNS]);

int main(int argc, char *argv[]) {
  char grid[ROWS][COLUMNS];
  int numeroColonna = 0, numeroRiga = 2;
  char movement;
  system("clear");
  fillGrid(0, 0, grid);
  printGrid(grid);
  while (1) {
    movement = getchar();
    switch (movement) {
    case 'w':
      numeroRiga--;
      numeroRiga = numeroRiga % ROWS;
      update(numeroColonna, numeroRiga, grid);
      break;

    case 's':
      numeroRiga++;
      numeroRiga = numeroRiga % ROWS;
      update(numeroColonna, numeroRiga, grid);
      break;

    case 'a':
      numeroColonna--;
      numeroColonna = numeroColonna % COLUMNS;
      update(numeroColonna, numeroRiga, grid);
      break;
    case 'd':
      numeroColonna++;
      numeroColonna = numeroColonna % COLUMNS;
      update(numeroColonna, numeroRiga, grid);
      break;
    default:
      break;
    }
  }
  return 0;
}
void update(int numeroColonna, int numeroRiga, char grid[][COLUMNS]) {
  fillGrid(numeroRiga, numeroColonna, grid);
  system("clear");
  printGrid(grid);
}
void fillGrid(int numeroRiga, int numeroColonna, char grid[][COLUMNS]) {
  int i = 0, j = 0;
  char a = 'a';
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      grid[i][j] = a++;
    }
  }
  grid[numeroColonna][numeroRiga] = '@';
}

void printGrid(char grigliaDaStampare[ROWS][COLUMNS]) {
  int i = 0, j = 0;
  for (i = 0; i < ROWS; i++) {
    for (j = 0; j < COLUMNS; j++) {
      printf("%c", grigliaDaStampare[i][j]);
    }
    printf("\n");
  }
}