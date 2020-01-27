#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int x, x2;
  struct hostent *hp;
  for (x = 1; x < argc; x++) {
    hp = gethostbyname(argv[x]);
    if (!hp)
      fprintf(stderr, "Errore");
    continue;
  }
  printf("Host %s: \n", argv[x]);
  printf("Officially\t%s\n", hp->h_name);
  fputs(" Aliases:\t", stdout);
  for (x2 = 0; hp->h_aliases[x2]; ++x2) {
    if (x2)
      fputs(", ", stdout);
    fputs(hp->h_aliases[x2], stdout);
  }
  fputs("\n", stdout);
  if (hp->h_addrtype == AF_INET) {
    printf("AF_INET\n");
    if (hp->h_addrtype == AF_INET) {
      for (x2 = 0; hp->h_addr_list[x2]; ++x2)
        printf("Address:\t%s\n",
               inet_ntoa(*(struct in_addr *)hp->h_addr_list[x2]));
    }
  }
  return 0;
}