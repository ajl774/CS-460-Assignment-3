#include <semaphore.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>

#define TARGET_ADDR "time-a.nist.gov"
#define TARGET_PORT "13"

#define BUF_SIZE 80
#define SERVER_ADDR "127.0.0.1" // loopback ip address
#define PORT 23867              // port the server will listen on

#define FALSE 0
#define TRUE !FALSE

#define NUM_CONNECTIONS 5       // number of pending connections in the connection queue

void* handle_client(void* arg);
