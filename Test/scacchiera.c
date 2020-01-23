#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
 
#define ROWS 7
#define COLUMNS 12
 /*
 	Ho eliminato la funzione update che chiamava fillGrid tante volte quante erano gli spostamenti, ma ci� impicava un aumento "consistente" della complessit�
 	poich� riempiva di continuo la griglia: basta invece conoscere la vecchia posizione del personaggio e sostituire alla sua immagine il flag '-', in questo modo
 	non necessito di una funzione per l'update che chiami ogni volta fillGrid(), che ora � diventato fillGridInitializer() poich� viene chiamato solo una volta all'apertura dell'app.
 	
 	Ho aggiunto un tasto d'uscita in maniera tale da finire "il test-gioco" una volta inserito il carattere "p".
 */
char grid[ROWS][COLUMNS];
int score = 0;
void fillGridInitializer();
void printGrid();
void start();
void printScore();
void printAll();
int main(int argc, char *argv[]){
    start();
    return 0;
}
void start(){
    char movement;
	int riga=0, colonna=0;
	fillGridInitializer();	
	grid[riga][colonna]='#';
	system("clear");
    printAll(); 
    while(1){
        movement=getchar();
        switch (movement)
        {
        case 'w':
            if((riga-1) >= 0){
				grid[riga][colonna]='-';
				riga = riga-1;
			}
            break;
        case 's':
            if((riga+1) < ROWS){
				grid[riga][colonna]='-';
				riga = riga+1;
            }
		    break;
        case 'a':
            if( (colonna-1) >= 0){
    			grid[riga][colonna]='-';
				colonna = (colonna-1)%COLUMNS;
        	}
			break;
        case 'd':
            if((colonna+1) < COLUMNS){
    			grid[riga][colonna]='-';
				colonna = (colonna+1)%COLUMNS;
        	}	
			break;
        default:
            break;
        }
        if(movement=='p'){
        	printf("+++++Game Over+++++\n\n");
        	break;
		}
        system("clear");
        grid[riga][colonna]='#';
    	printAll();
    }
}

 
void printAll(){
    printScore();
    printGrid();
}
void fillGridInitializer(){
    int i=0, j=0;
    for ( i = 0; i < ROWS; i++){
       for ( j = 0; j < COLUMNS; j++){
           grid[i][j]='-';
       }
    }
}
void printScore(){
    printf("\t SCORE: %d\n", score);
}
void printGrid(){
    int i=0, j=0;
    for( i=0;i<ROWS;i++){
        printf("\t");
       for(j=0; j<COLUMNS; j++){
           printf("%c", grid[i][j]);
       }
       printf("\n");
    }
}
