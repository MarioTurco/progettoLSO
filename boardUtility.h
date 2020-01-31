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
void inizializzaGiocoSenzaPlayer(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaConOstacoli[ROWS][COLUMNS]);
void inserisciPlayerNellaGrigliaInPosizioneCasuale(
    char grigliaDiGioco[ROWS][COLUMNS], char grigliaOstacoli[ROWS][COLUMNS],
    int posizione[2]);
void inizializzaGrigliaVuota(char grigliaDiGioco[ROWS][COLUMNS]);
void generaPosizioneOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS]);
void riempiGrigliaConPacchiInPosizioniGenerateCasualmente(
    char grigliaDiGioco[ROWS][COLUMNS]);
void printGrid(char grigliaDaStampare[ROWS][COLUMNS]);
void start(char grigliaDiGioco[ROWS][COLUMNS],
           char grigliaOstacoli[ROWS][COLUMNS]);
void printObs(char grigliaOstacoli[ROWS][COLUMNS]);
void riempiGrigliaConGliOstacoli(char grigliaDiGioco[ROWS][COLUMNS],
                                 char grigliaOstacoli[ROWS][COLUMNS]);
void gestisciInput(char grigliaDiGioco[ROWS][COLUMNS],
                   char grigliaOstacoli[ROWS][COLUMNS], int posizioneUtente[2],
                   int destinazione[2], char input, int *punteggio);
void generaPosizioneRaccolta(char grigliaDiGioco[ROWS][COLUMNS],
                             char grigliaOstacoli[ROWS][COLUMNS], int *coord,
                             int xPlayer, int yPlayer);
PlayerStats gestisciW(char grigliaDiGioco[ROWS][COLUMNS],
                      char grigliaOstacoli[ROWS][COLUMNS],
                      PlayerStats giocatore, Obstacles *listaOstacoli);
void mergeGridAndList(char grid[ROWS][COLUMNS], Obstacles top);
int colpitoOstacolo(char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]);
int colpitoPacco(char grigliaDiGioco[ROWS][COLUMNS], int posizione[2]);
int colpitoPlayer(char grigliaDiGioco[ROWS][COLUMNS], int posizione[2]);
int casellaVuota(char grigliaDiGioco[ROWS][COLUMNS],
                 char grigliaOstacoli[ROWS][COLUMNS], int posizione[2]);
void aggiornaGrigliaW(char griglia[ROWS][COLUMNS], int vecchiaPosizione[2],
                      int nuovaPosizione[2]);
int arrivatoADestinazione(int posizione[2], int destinazione[2]);