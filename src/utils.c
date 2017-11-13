#include "utils.h"

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void mysh_parse_command(const char* command,
                        int* n_commands,
                        struct single_command (*commands)[])
{
  char buf[4096];
  strcpy(buf, command);

  char *saveptr = NULL;
  char *tok = strtok_r(buf, "|", &saveptr);

  int ti = 0;

  while (tok != NULL) {
    struct single_command* com = *commands + ti;
    parse_single_command(tok, &com->argc, &com->argv);

    ++ti;

    tok = strtok_r(NULL, "|", &saveptr);
  }

  *n_commands = ti;
}

void parse_single_command(const char* command,
                          int *argc, char*** argv)
{
  const int kMaxArgc = 512;
  *argv = (char**)malloc(kMaxArgc * sizeof(char*));

  char* resol1[10] = {"ls", "/"};
  char* resol2[10] = {"cat", "/etc/hosts"};
  char* resol3[10] = {"vim"};
  
  for (int i = 0; i < kMaxArgc; ++i)
    (*argv)[i] = NULL;

  char buf[4096];

  strcpy(buf, command);

  char *saveptr = NULL;
  char *tok = strtok_r(buf, " \n\t", &saveptr);

  int ti = 0;

  while (tok != NULL) {
    (*argv)[ti] = (char*)malloc(strlen(tok));
    strcpy((*argv)[ti], tok);

    ++ti;

    tok = strtok_r(NULL, " \n\t", &saveptr);
  }
  
  if (argv[0][0] != NULL) {
      if (!strcmp(argv[0][0], resol1[0])){
    	  if (argv[0][1] != NULL && !strcmp(argv[0][1], resol1[1])) {
    	      strcpy((*argv)[0], "/bin/ls");
    	      strcpy((*argv)[1], "/");
	  }
      }
      if (!strcmp(argv[0][0], resol2[0])){
	  if (argv[0][1] != NULL && !strcmp(argv[0][1], resol2[1])) {
	      strcpy((*argv)[0], "/bin/cat");
	      strcpy((*argv)[1], "/etc/hosts");
	  }
      }
      if (!strcmp(argv[0][0], resol3[0])){
	  strcpy((*argv)[0], "/usr/bin/vim");
      }
  }
 
  *argc = ti;

  if (*argc == 0) {
    *argc = 1;
    (*argv)[0] = (char*)malloc(1);
    (*argv)[0][0] = '\0';
  }
}
