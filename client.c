#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define TARGET_ADDR "127.0.0.1"
#define TARGET_PORT "23867"

#define BUF_SIZE 80

int main(int argc, char *argv[]) {
    struct addrinfo hint, *result, *tmp;
    struct sockaddr_in *addr;
    char addr_str[INET_ADDRSTRLEN];
    char buf[BUF_SIZE];
    char c = 0;
    int sock_fd;
  
    bzero(&hint, sizeof(struct addrinfo));
  
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
  
    if (getaddrinfo(TARGET_ADDR, TARGET_PORT, &hint, &result)) {
        fprintf(stderr, "Failed to get address info.\n");
        return EXIT_FAILURE;
    }

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Failed to create socket\n");
        return EXIT_FAILURE;
    }
  
    if (connect(sock_fd, result->ai_addr, result->ai_addrlen)) {
        fprintf(stderr, "Failed to connect to server\n");
        return EXIT_FAILURE;
    }
  
    while (c != '*') {
        read(sock_fd, &c, sizeof(char));
        putchar(c);
    }
    printf("\n");
 
    close(sock_fd);
    freeaddrinfo(result);
  
    return EXIT_SUCCESS;
}
