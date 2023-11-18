#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string>
#include <set>
#include "service.hpp"
#include "entity.hpp"

using namespace std;

#define PORT 8080
#define BUFF_SIZE 1024

struct ClientInfo {
    int socket;
    Account *account;
    int status;

    ClientInfo() {}

    ClientInfo(int socket, Account *account) :
            socket(socket), account(account) {}

    bool operator<(const ClientInfo &other) const {
        return account->username < other.account->username;
    }
};

set <ClientInfo> clients;


int findSocketByUsername(string username) {
    for (const auto &client: clients) {
        if (client.account->username == username) {
            return client.socket;
        }
    }
    return -1;
}

void handleSend(int clientSocket, char *messageType, char *payload) {
    char buff[BUFF_SIZE];
    int bytesSent;
    int blockTypeSize = 4;
    int headerSize = strlen(messageType) + blockTypeSize + 2;

    while (headerSize + strlen(payload) > BUFF_SIZE - 1) {
        memset(buff, 0, BUFF_SIZE);
        char *payloadSubstring;
        strncpy(payloadSubstring, payload, BUFF_SIZE - 1 - headerSize);
//        sprintf(buff, "%s %s\n%s", messageType, "MID ", payloadSubstring);

        bytesSent = send(clientSocket, buff, strlen(buff), 0);

        payload = payload + (BUFF_SIZE - 1 - headerSize);
    }

    memset(buff, 0, BUFF_SIZE);
    sprintf(buff, "%s %s\n%s", messageType, "LAST", payload);
    bytesSent = send(clientSocket, buff, strlen(buff), 0);
    printf("Sent:\n%s\n", buff);
}

void notifyOnlineStatusChange() {
    for (const auto &client: clients) {
        handleSend(client.socket, "CHGONL", "");
    }
}

void handleClientDisconnect(int clientSocket) {
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if (it->socket == clientSocket) {
            clients.erase(it);
            notifyOnlineStatusChange();
            return;
        }
    }
}

void *handleReceive(void *arg) {
    int clientSocket = *(int *) arg;

    while (1) {
        char buff[BUFF_SIZE];
        char *messageType, *blockType, payload[32 * BUFF_SIZE];
        payload[0] = '\0';

        do {
            int bytesReceived = recv(clientSocket, buff, BUFF_SIZE - 1, 0);
            if (bytesReceived <= 0) {
                printf("Client disconnected.\n");
                handleClientDisconnect(clientSocket);
                return NULL;
            }

            printf("buff = %s\n", buff);

            int headerEndIndex = strcspn(buff, "\n");
            buff[headerEndIndex] = '\0';

            int splitIndex = strcspn(buff, " ");
            buff[splitIndex] = '\0';

            messageType = buff;
//            printf("messageType = %s\n", messageType);
            blockType = buff + splitIndex + 1;
//            printf("blockType = %s\n", blockType);
            if (buff[headerEndIndex + 1] != '\0')
                strcat(payload, buff + headerEndIndex + 1);
//            printf("payload = %s\n", payload);

        } while (strcmp(blockType, "LAST") != 0);

        printf("Received:\n%s\n", payload);

        if (strcmp(messageType, "SIGNUP") == 0) {
            int res = doCreateAccount(payload);
            switch (res) {
                case 0:
                    handleSend(clientSocket, "OK", "Account created successfully");
                    break;
                case 1:
                    handleSend(clientSocket, "ERROR", "Username already used");
                    break;
            }
        } else if (strcmp(messageType, "SIGNIN") == 0) {
            Account *account = doSignIn(payload);
            if (account == NULL) {
                handleSend(clientSocket, "ERROR", "Wrong username or password");
            } else {
                handleSend(clientSocket, "OK", "Signed in successfully");
                clients.insert(ClientInfo(clientSocket, account));
                notifyOnlineStatusChange();
            }
        } else if (strcmp(messageType, "LSTONL") == 0) {
            payload[0] = '\0';
            char content[BUFF_SIZE];

            for (const auto &client: clients) {
                char status[20];
                if (client.status == 0) {
                    strcpy(status, "Available");
                } else {
                    strcpy(status, "In game");
                }

                sprintf(content, "%s %s\n", client.account->username.c_str(), status);
                strcat(payload, content);
            }

            handleSend(clientSocket, "LSTONL", payload);
        }
    }

    return NULL;
}

void *handleClient(void *arg) {
    int clientSocket = *(int *) arg;
    pthread_t receiveThread;
    pthread_create(&receiveThread, NULL, handleReceive, (void *) &clientSocket);
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

    if (listen(serverSocket, 5) == -1) {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d.\n", PORT);

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr *) &client, &sinSize);
        printf("Connection from %s.\n", inet_ntoa(client.sin_addr));

        pthread_t clientThread;
        pthread_create(&clientThread, NULL, handleClient, (void *) &clientSocket);
    }

    close(serverSocket);

    return 0;
}
