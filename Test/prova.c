#include"parser.h"
#include<stdio.h>


int main(int argc, char**argv){
    appendPlayer("azankiew","machevuoi98",argv[1]);
    appendPlayer("basta","s",argv[1]);
    appendPlayer("vabe","e",argv[1]);
    appendPlayer("boi","rr",argv[1]);
    appendPlayer("ew","t",argv[1]);
    appendPlayer("ao","g",argv[1]);
    printf("%d\n", isRegistered("boi",argv[1]));
    printf("%d\n", isRegistered("ao",argv[1]));
    printf("%d\n", isRegistered("azankiew",argv[1]));
    printf("%d\n", isRegistered("mamt",argv[1]));
    printf("L'utente azankiew è loggato? %d\n",validateLogin("azankiew","machevuoi98",argv[1]));//si
    printf("L'utente aeEeEeEeE è loggato? %d\n",validateLogin("aeEeEeEeE","machevuoi98",argv[1]));//non registrato ancora
    printf("L'utente azankiew è loggato? %d\n",validateLogin("azankiew","machevuoi",argv[1]));//credenziali errate ma registrato

    return 0;
}