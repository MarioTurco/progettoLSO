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

void inizializzaGrigliaVuota(char grigliaDiGioco[ROWS][COLUMNS]);
void generaPosizioneOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[numberOfObstacles][2]);
void generatePackagesPositions(char grigliaDiGioco[ROWS][COLUMNS],
                               char grigliaPacchi[numberOfPackages][2]);
void printGrid(char grigliaDaStampare[ROWS][COLUMNS]);
void start(char grigliaDiGioco[ROWS][COLUMNS],
           char grigliaOstacoli[numberOfObstacles][2],
           char grigliaPacchi[numberOfPackages][2]);
void gameOver();
void printObs(char grigliaOstacoli[numberOfObstacles][2]);
void riempiGrigliaConGliOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaOstacoli[numberOfObstacles][2]);
