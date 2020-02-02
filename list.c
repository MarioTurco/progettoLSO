#include "list.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Players initPlayerNode(char *name, int sockDes) {
  Players L = (Players)malloc(sizeof(struct TList));
  L->name = (char *)malloc(MAX_BUF);
  strcpy(L->name, name);
  L->sockDes = sockDes;
  L->next = NULL;
  return L;
}

PlayerStats initStats(int deploy[], int score, int position[]) {
  PlayerStats L = (PlayerStats)malloc(sizeof(struct Data));
  L->deploy[0] = deploy[0];
  L->deploy[1] = deploy[1];
  L->score = score;
  L->position[0] = position[0];
  L->position[1] = position[1];
  return L;
}

Obstacles initObstacleNode(int x, int y) {
  Obstacles L = (Obstacles)malloc(sizeof(struct TList2));
  L->x = x;
  L->y = y;
  L->next = NULL;
  return L;
}

Obstacles addObstacle(Obstacles L, int x, int y) {
  Obstacles tmp = initObstacleNode(x, y);
  if (L != NULL) {
    tmp->next = L;
  }
  return tmp;
}

int isAlreadyLogged(Players L, char *name) {
  int ret = 0;
  if (L != NULL) {
    if (strcmp(L->name, name) == 0)
      return 1;
    ret = isAlreadyLogged(L->next, name);
  }
  return ret;
}

Players addPlayer(Players L, char *name, int sockDes) {
  Players tmp = initPlayerNode(name, sockDes);
  if (L != NULL) {
    tmp->next = L;
  }
  return tmp;
}

Players removePlayer(Players L, int sockDes) {
  if (L != NULL) {
    if (L->sockDes == sockDes) {
      Players tmp = L->next;
      free(L);
      return tmp;
    }
    L->next = removePlayer(L->next, sockDes);
  }
  return L;
}

void freePlayers(Players L) {
  if (L != NULL) {
    freePlayers(L->next);
    free(L);
  }
}

void freeObstacles(Obstacles L) {
  if (L != NULL) {
    freeObstacles(L->next);
    free(L);
  }
}

void printPlayers(Players L) {
  if (L != NULL) {
    printf("%s ->", L->name);
    printPlayers(L->next);
  }
}

void printObstacles(Obstacles L) {
  if (L != NULL) {
    printf("X:%d Y:%d ->", L->x, L->y);
    printObstacles(L->next);
  }
}