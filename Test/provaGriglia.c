#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

#define ROWS 7
#define COLUMNS 12

char grid[ROWS][COLUMNS];
void update(int nuovaRiga, int nuovaColonna );
void fillGrid();
void printGrid();
void start();

int main(int argc, char *argv[]){
    start(grid);
    return 0;
}

void start(char grid[][COLUMNS]){
    char movement;
    int riga=0, colonna=0;
    update(riga, colonna);
    while(1){
        movement=getchar();
        switch (movement)
        {
        case 'w':
            if((riga-1) >= 0)
                riga = riga-1;
            break;
        case 's':
            if((riga+1) < ROWS)
                riga = riga+1;
            break;
        case 'a':
            if( (colonna-1) >= 0)
                colonna = (colonna-1)%COLUMNS;
            break;
        case 'd':
            if((colonna+1) < COLUMNS)
                colonna = (colonna+1)%COLUMNS;
            break;
        default:
            break;
        }
        update(riga, colonna);
    }
}
void update(int nuovaRiga, int nuovaColonna){
    system("clear");
    fillGrid(grid);
    grid[nuovaRiga][nuovaColonna]='#';
    printGrid(grid);
}

void fillGrid(){
    int i=0, j=0;
    for ( i = 0; i < ROWS; i++){
       for ( j = 0; j < COLUMNS; j++){
           grid[i][j]='-';
       }
    }
}

void printGrid(){
    int i=0, j=0;
    for( i=0;i<ROWS;i++){
       for(j=0; j<COLUMNS; j++){
           printf("%c", grid[i][j]);
       }
       printf("\n");
    }
}