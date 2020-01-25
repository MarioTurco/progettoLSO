#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ROWS 20
#define COLUMNS 65
#define numberOfObstacles 100
#define numberOfPackages 30
#define timeLimitInSeconds 240
#define packageLimitNumber 8

void fillGridInitializer();
void generateObstalePositions();
void generatePackagesPositions();
void printGrid(char grigliaDaStampare[ROWS][COLUMNS]);
void start();
void printAll();
void gameOver();
void printMenu();
void printObs();