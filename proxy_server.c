#include "proxy_server.h"






sem_t mutex;

int main(int argc, char** argv)
{
    int server_socket;                 // descriptor of server socket
    struct sockaddr_in server_address; // for naming the server's listening socket
    int yes = 1;
    


    // ----------------------------------------------------------
    // ignore SIGPIPE, sent when client disconnected
    // ----------------------------------------------------------
    signal(SIGPIPE, SIG_IGN);
    
    // ----------------------------------------------------------
    // create unnamed network socket for server to listen on
    // ----------------------------------------------------------
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    printf("\nCreated socket, %d, to listen on\n",server_socket);    
    // lose the pesky "Address already in use" error message
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // ----------------------------------------------------------
    // bind the socket
    // ----------------------------------------------------------
    server_address.sin_family      = AF_INET;           // accept IP addresses
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // accept clients on any interface
    server_address.sin_port        = htons(PORT);       // port to listen on
    
    // binding unnamed socket to a particular port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0) 
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    printf("Server socket bound to port #%d\n", PORT);
    // ----------------------------------------------------------
    // listen on the socket
    // ----------------------------------------------------------
    if (listen(server_socket, NUM_CONNECTIONS) != 0)
    {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port #%d\n", PORT);
    
    //Using a binary semaphore to stop race conditions
    sem_init(&mutex, 0, 1);
    
    // ----------------------------------------------------------
    // server loop
    // ----------------------------------------------------------
    printf("waiting for client...\n");
    while (TRUE)
    {
        //sem wait so we don't overwrite the client socket before the thread is finished.
        sem_wait(&mutex);
      	  
        // accept connection to client
        int client_socket = accept(server_socket, NULL, NULL);
        
	printf("Server with PID %d: accepted client\n", getpid());
 	         
        
	
        //crtical section over.
        
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void*)&client_socket) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
        
        
        // detach the thread so that we don't have to wait (join) with it to reclaim memory.
        // memory will be reclaimed when the thread finishes.
        if (pthread_detach(thread) != 0)
        {
            perror("Error detaching thread");
            exit(EXIT_FAILURE);
        }
	
    }
    
    sem_destroy(&mutex);
    
}



void* handle_client(void* arg) 
{
    printf("Handling client...\n");
    struct addrinfo hint, *result, *tmp;
    struct sockaddr_in *addr;
    char addr_str[INET_ADDRSTRLEN];
    char buf[BUF_SIZE];
    char c = 0;
    int sock_fd;

    //unpack client socket
    int client_socket = *(int*)arg;

    sem_post(&mutex);

    bzero(&hint, sizeof(struct addrinfo));
  
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    
    
    if (getaddrinfo(TARGET_ADDR, TARGET_PORT, &hint, &result)) {
        perror("Failed to get address info.\n");
        exit(EXIT_FAILURE);
        
    }
    printf("Getting address info for NIST server...\n");
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Creating socket to listen for NIST data...\n");
    if (connect(sock_fd, result->ai_addr, result->ai_addrlen)) {
        perror("Failed to connect to server\n");
        exit(EXIT_FAILURE);
    }
    printf("Connecting to NIST server...\n");
    int count = 0;
    printf("Reading data from NIST server...\n");
    while (c != '*' && count < BUF_SIZE - 1) {
        read(sock_fd, &c, sizeof(char));
        buf[count] = c;
        count++;
    }
    buf[count] = '\0';
    printf("Writing data to client...\n");
    write(client_socket, &buf, strlen(buf));
    

    if (close(client_socket) == -1) 
    { 
	perror("Error closing socket");
        exit(EXIT_FAILURE);
    } 
    else
    {
        printf("Closed socket to client, exit\n");
	    printf("\nWaiting for new client...\n");
    }
 
    close(sock_fd);
    freeaddrinfo(result);
    pthread_exit(NULL);
}




