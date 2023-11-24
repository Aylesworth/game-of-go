#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string>
#include <set>
#include "controller.hpp"
#include "service.hpp"
#include "entity.hpp"
#include "go_engine.hpp"

using namespace std;

#define PORT 8080

void *handleClient(void *arg) {
    pthread_detach(pthread_self());

    int clientSocket = *(int *) arg;
    pthread_t receiveThread;
    pthread_create(&receiveThread, NULL, handleRequest, (void *) &clientSocket);
    pthread_join(receiveThread, NULL);

    close(clientSocket);

    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in server, client;
    socklen_t sinSize = sizeof(struct sockaddr_in);

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.sin_zero), 8);

    if (bind(serverSocket, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("Error binding to socket");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 100) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d.\n", PORT);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *) &client, &sinSize);
        printf("Connection from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handleClient, (void *) &clientSocket);
    }

    close(serverSocket);

    return 0;
}
