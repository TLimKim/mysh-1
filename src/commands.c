#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h> // for fork(), execv()
#include <sys/types.h>
#include <sys/wait.h> // wait()
#include <sys/socket.h> // bind(), 
#include <sys/un.h> // structure for socket
#include <errno.h> // for error Number
#include <pthread.h> // thread management

#include "commands.h"
#include "built_in.h"

#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512]);

void* client_thread(void* threadData) {
    char** parse_com = ((char **)threadData);
    int ti = 0;

    for(int i = 0; parse_com[i] != NULL; i++) {
	ti++;
    }

    struct single_command t_com[512] = {ti, parse_com};
    
    int c_sock, rc, len;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;
    char buf[256];
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));

    // Create Socket
    c_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (c_sock == -1) {
	printf("SOCKET ERROR(client) \n");
	exit(1);
    }

    // Set up ~
    //
    // Unlink the file
    client_sockaddr.sun_family = AF_UNIX;
    strcpy(client_sockaddr.sun_path, CLIENT_PATH);
    len = sizeof(client_sockaddr);
    
    /*
    unlink(CLIENT_PATH);
    rc = bind(c_sock, (struct sockaddr *) &client_sockaddr, len);
    if (rc == -1) {
	printf("BIND ERROR(Client) \n");
	close(c_sock);
	exit(1);
    }*/
    
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    rc = connect(c_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1) {
	printf("CONNECT ERROR\n");
	close(c_sock);
	exit(1);
    }
    printf("Connect Success\n");
    
    sleep(3);
    
    dup2(c_sock, 1);
    close(c_sock);

    evaluate_command(1, &t_com);

    sleep(3);
    close(c_sock);
    pthread_exit(NULL);
}

int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
    struct single_command* com = (*commands);

    assert(com->argc != 0);
    
    /*
    for (int i = 0; i < 10; i++) {
	printf("%s ", com->argv[i]);
    }
    printf("\n");

    printf("%s\n", (com+1)->argv[0]);
    */

    if (n_commands == 1) {
	int built_in_pos = is_built_in_command(com->argv[0]);

	if (built_in_pos != -1) {
	  if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
	    if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
	      fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
	    }
	  } else {
	    fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
	    return -1;
	  }
	} else if (strcmp(com->argv[0], "") == 0) {
	  return 0;
	} else if (strcmp(com->argv[0], "exit") == 0) {
	  return 1;
	} else {
	    //  Process Creation Part	   
	    pid_t child_pid;
	    int child_status;

	    child_pid = fork();

	    if (child_pid == 0) {

		execv(com->argv[0], com->argv);
		fprintf(stderr, "%s: command not found\n", com->argv[0]);
		exit(0);	
	    } else {
		    wait(&child_status);
		    return 0;
	    }
	}
  } else if (n_commands >= 2) {
      
      //socket start
      int s_sock, c_sock, len, rc;
      int backlog = 10;
  
      struct sockaddr_un s_sockaddr;
      struct sockaddr_un c_sockaddr;

      memset(&s_sockaddr, 0, sizeof(struct sockaddr_un));
      memset(&c_sockaddr, 0, sizeof(struct sockaddr_un));
  
      // Creating a UNIX domain stream socket
      s_sock = socket(AF_UNIX, SOCK_STREAM, 0);
      if (s_sock == -1) {
	  printf("SOCKET ERROR\n");
	  exit(1);
      } 
      // Set up the UNIX sockaddr structure by using AF_UNIX for the family and
      // giving it a file path to bind to.
      //
      // Unlink the file so the bind will succeed, then bind to that file.
      s_sockaddr.sun_family = AF_UNIX;
      strcpy(s_sockaddr.sun_path, SERVER_PATH);
      len = sizeof(s_sockaddr);

      unlink(SERVER_PATH);
      rc = bind(s_sock, (struct sockaddr *) &s_sockaddr, len);
     
      if (rc == -1) {
  	  printf("BIND ERROR\n");
  	  close(s_sock);
  	  exit(1);
      }

      // Listening for any client sockets
      rc = listen(s_sock, backlog);
      if (rc == -1) {
	  printf("LISTEN ERROR\n");
  	  close(s_sock);
  	  exit(1);
      }
      printf("binding success & socket listening... \n");

      // Threading for client starts now
      pthread_t threads[5];
      int status;

      rc = pthread_create(&threads[0], NULL, client_thread, (void*)com->argv);
      if (rc == -1) {
	  printf("Creating Thread Error\n");
	  exit(0);
      }
      printf("Create Thread Success\n");

      // Waiting until Thread is exit
      //pthread_join(threads[0], (void**)&status);

      /*
      c_sock = accept(s_sock, (struct sockaddr *) &c_sockaddr, &len);
      if (C_sock == -1) {
	  printf("Accepting Error \n");
	  exit(1);
      }
      printf("Accepting Clear \n");
*/
      while (1) {
      
      if ((c_sock = accept(s_sock, (struct sockaddr *) &c_sockaddr, &len)) == -1) {
	      printf("Accepting Error\n");
	      exit(1);
	  }
	  printf("Accepting Clear\n");	  
          pid_t child_pid;
          int child_status;
	  
	  child_pid = fork();
	  
	  if (child_pid < 0) {
      	      printf("Folk Failed\n");
	      exit(1);
	  } else if (child_pid == 0) {
	      dup2(c_sock, STDIN_FILENO);
	      printf("duped passed \n");
	      close(c_sock);
	      execv((com+1)->argv[0], (com+1)->argv);
	      fprintf(stderr, "%s: command not found\n", (com+1)->argv[0]);
	  }
	  else {
	      close(c_sock);
	      wait(&child_status);
	      return 0;
	  }
	  pthread_join(threads[0], (void**)&status);
      }
      //pthread_join(threads[0], (void**)&status);
      close(s_sock);
      close(c_sock);
      return 0;
  }
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
