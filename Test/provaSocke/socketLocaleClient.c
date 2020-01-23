#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define UNIX_PATH_MAX 108

static int client(void);

int main()
{
    return client();
    /*struct sockaddr mio_indirizzo;
    int fileDesc1;
    
    //impostiamo la famiglia ed il path degli indirizzi
    mio_indirizzo.sa_family= AF_LOCAL; 
    strcpy(mio_indirizzo.sa_data, "/tmp/mio_socket");

    //creo il socket e setto famiglia e tipo e protocollo
    fileDesc1 = socket(PF_LOCAL, SOCK_STREAM, 0);
    
    connect(fileDesc1, (struct sockaddr*)&mio_indirizzo, sizeof(mio_indirizzo));
    
    close(fileDesc1);*/

    return 0;
}
int client(void)
{
    char msg[100];
    struct sockaddr serverAddress;
    int serverDescriptor, i;
    ssize_t temp;

    signal(SIGPIPE, SIG_IGN);

    //crea il socket
    serverDescriptor = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (serverDescriptor < 0)
    {
        perror("socket");
        exit(-1);
    }
    serverAddress.sa_family = AF_LOCAL;
    strcpy(serverAddress.sa_data, "/tmp/mySocket");

    //si connette all'indirizzo

    if (connect(serverDescriptor, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("connect");
        exit(-1);
    }
    sleep(0.5);
    read(serverDescriptor, msg, 26);
    printf("Client riceve: *%s*\n", msg);
    printf(" client riceve: %ld\n", strlen(msg));
    return 0;
}