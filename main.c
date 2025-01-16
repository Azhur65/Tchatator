#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define STRING_SIZE 200

#define CLIENT_FILE "client.csv"

int main() {
	int sock, ret, cnx, reading, fd, writing;
    int size;
    struct sockaddr_in conn_addr, addr;
    char buffer[BUFFER_SIZE], reponse[STRING_SIZE];
    size = sizeof(conn_addr);

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error creating socket");
        exit(1);
    }
    
    // Set up the address structure for binding
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);  // Port 8080
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        close(sock);
        exit(1);
    }
    
    // Bind socket to the address and port
    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("Error binding socket");
        close(sock);
        exit(1);
    }
    
    // Start listening for incoming connections
    ret = listen(sock, 1);
    if (ret < 0) {
        perror("Error listening on socket");
        close(sock);
        exit(1);
    }

    printf("Server listening on 127.0.0.1:8080...\n");

    // Accept incoming connection
    cnx = accept(sock, (struct sockaddr *)&conn_addr, (socklen_t *)&size);
    if (cnx < 0) {
        perror("Error accepting connection");
        close(sock);
        exit(1);
    }

    printf("Connection established\n");

	do{

		// Read 
		reading = read(cnx,buffer,BUFFER_SIZE);
		if(reading < 0){
			perror("Error on reading \n");
			close(sock);
			exit(1);
		}else if(reading == 0){
			perror("Nothing has been read\n");
			close(sock);
			exit(1);
		}else{
			printf("%d bytes has been read \n",reading);
		}

		scanf(%s,message);

		// Write
		writing = write(cnx, buffer);
		if(writing < 0){
			perror("Error on writing \n");
			close(sock);
			exit(1);
		}else if(writing == 0){
			perror("Nothing has been written\n");
			close(sock);
			exit(1);
		}else{
			printf("%d bytes has been written \n",writing);
		}


	}while();


    // Close the connection and the listening socket
    close(cnx);
    close(sock);


	return EXIT_SUCCESS;
}
