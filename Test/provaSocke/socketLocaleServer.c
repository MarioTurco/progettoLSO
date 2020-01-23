#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#define UNIX_PATH_MAX 108

static int gestisci(int fileDescClient);
int main()
{

    struct sockaddr mio_indirizzo;
    struct sockaddr client_addr;
    socklen_t client_len;
    int listen_socketDesc, connect_socketDesc;

    //impostiamo la famiglia ed il path degli indirizzi
    mio_indirizzo.sa_family = AF_LOCAL;
    strcpy(mio_indirizzo.sa_data, "/tmp/mySocket");

    //creo il socket e setto famiglia e tipo e protocollo
    if (listen_socketDesc = socket(PF_LOCAL, SOCK_STREAM, 0) < 0)
    {
        perror("Socket");
        exit(-1);
    }
    // Rimuovi il socket se esiste già
    unlink("/tmp/mio_socket");
    //assegno l'indirizzo al socket
    if (bind(listen_socketDesc, (struct sockaddr *)&mio_indirizzo, sizeof(mio_indirizzo)) < 0)
    {
        perror("Bind");
        exit(-1);
    }
    //ti metti in ascolto con una coda di 3 client
    if (listen(listen_socketDesc, 1) < 0)
    {
        perror("Listen");
        exit(-1);
    }
    while (1)
    {
        client_len = sizeof(client_addr);
        fprintf(stderr, "sizeof(client_addr)= %d\n", client_len);

        if ((connect_socketDesc = accept(listen_socketDesc, (struct sockaddr *)&client_addr, &client_len)) < 0)
        {
            perror("Connessione");
            exit(-1);
        }
        fprintf(stderr, " new connection \n");
        fprintf(stderr, " client address: %100s\n ", client_addr.sa_data);

        //gestisci la connessione
        gestisci(connect_socketDesc);
        close(connect_socketDesc);
    }

    close(listen_socketDesc);
    close(connect_socketDesc);
    unlink("/tmp/mio_socket");

    return 0;
}

static int gestisci(int fileDescClient)
{
    char buffer[26];
    int n;
    time_t ora;
    time(&ora);
    printf("Ora: %s\n", ctime_r(&ora, buffer));
    printf("Ora: %ld\n", strlen(buffer));
    write(fileDescClient, buffer, 26);
    return 0;
}