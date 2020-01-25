#include "parser.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_BUF 200

int openFileRDWRAPP(char *file) {
  int fileDes = open(file, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

  if (fileDes < 0) {
    perror("error while opening file");
    exit(-1);
  }
  return fileDes;
}

int openFileRDON(char *file) {
  int fileDes = open(file, O_RDONLY);

  if (fileDes < 0) {
    perror("error while opening file");
    exit(-1);
  }
  return fileDes;
}

int appendPlayer(char *name, char *pwd, char *file) {
  if (isRegistered(name, file))
    return 0;
  int fileDes = openFileRDWRAPP(file);

  write(fileDes, name, strlen(name));
  write(fileDes, " ", 1);
  write(fileDes, pwd, strlen(pwd));
  write(fileDes, "\n", 1);

  close(fileDes);
  return 1;
}

int isRegistered(char *name, char *file) {

  char command[MAX_BUF] = "cat  ";
  strcat(command, file);
  char toApp[] = " |cut -d\" \" -f1|grep \"^";
  strcat(command, toApp);
  strcat(command, name);
  char toApp2[] = "$\">tmp";
  strcat(command, toApp2);
  int ret = 0;
  system(command);
  int fileDes = openFileRDON("tmp");

  if (fileDes < 0) {
    perror("Error while opening file");
    exit(-1);
  }
  struct stat info;
  fstat(fileDes, &info);
  if ((int)info.st_size > 0)
    ret = 1;

  close(fileDes);
  system("rm tmp");
  return ret;
}

int validateLogin(char *name, char *pwd, char *file) {
  if (!isRegistered(name, file))
    return 0;

  char command[MAX_BUF] = "cat  ";
  strcat(command, file);
  char toApp[] = " |cut -d\" \" -f2|grep \"^";
  strcat(command, toApp);
  strcat(command, pwd);
  char toApp2[] = "$\">tmp";
  strcat(command, toApp2);
  int ret = 0;
  system(command);
  int fileDes = openFileRDON("tmp");
  if (fileDes < 0) {
    perror("Error while opening file");
    exit(-1);
  }
  struct stat info;
  fstat(fileDes, &info);
  if ((int)info.st_size > 0)
    ret = 1;

  close(fileDes);
  system("rm tmp");
  return ret;
}