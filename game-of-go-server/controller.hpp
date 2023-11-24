//
// Created by Admin on 11/21/2023.
//

#ifndef GAME_OF_GO_SERVER_CONTROLLER_HPP
#define GAME_OF_GO_SERVER_CONTROLLER_HPP

#include "service.hpp"
#include "go_engine.hpp"
#include <pthread.h>
#include <set>
#include <map>

using namespace std;

#define BUFF_SIZE 1024

int handleSend(int, const char *, const char *);

int handleReceive(int, char *, char *);

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

set<ClientInfo *> clients;

map<int, GoGame *>
        games;

ClientInfo *findClientByUsername(string username) {
    for (const auto &client: clients) {
        if (client->account->username == username) {
            return client;
        }
    }
    return NULL;
}

void notifyOnlineStatusChange() {
    for (const auto &client: clients) {
        handleSend(client->socket, "CHGONL", "");
    }
}

void handleClientDisconnect(int clientSocket) {
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if ((*it)->socket == clientSocket) {
            clients.erase(it);
            notifyOnlineStatusChange();
            return;
        }
    }
}

int generateKey(int socket1, int socket2) {
    int h1 = hash < int > {}(socket1);
    int h2 = hash < int > {}(socket2);
    int key = (h1 + h2) ^ ((h1 - h2) & (h2 - h1)) ^ (h1 * h2) ^ (h1 / h2) ^ (h2 / h1) ^ (h1 % h2) ^ (h2 % h1);
    return key;
}

int handleSend(int clientSocket, const char *messageType, const char *payload) {
    char buff[BUFF_SIZE];
    int bytesSent;
    int blockTypeSize = 4;
    int headerSize = strlen(messageType) + blockTypeSize + 2;

    while (headerSize + strlen(payload) > BUFF_SIZE - 1) {
        memset(buff, 0, BUFF_SIZE);
        char *payloadSubstring;
        strncpy(payloadSubstring, payload, BUFF_SIZE - 1 - headerSize);
        sprintf(buff, "%s %s %d\n%s", messageType, "MID ", BUFF_SIZE - 1 - headerSize, payloadSubstring);

        bytesSent = send(clientSocket, buff, strlen(buff), 0);
        if (bytesSent < 0) {
            return 0;
        }

        payload = payload + (BUFF_SIZE - 1 - headerSize);
    }

    memset(buff, 0, BUFF_SIZE);
    sprintf(buff, "%s %s %ld\n%s", messageType, "LAST", strlen(payload), payload);
    bytesSent = send(clientSocket, buff, strlen(buff), 0);
    printf("Sent to %d:\n%s\n", clientSocket, buff);

    return 1;
}

int handleReceive(int clientSocket, char *messageType, char *payload) {
    char buff[BUFF_SIZE];
    char *blockType;
    payload[0] = '\0';

    memset(payload, 0, 16 * BUFF_SIZE);
    do {
        memset(buff, 0, BUFF_SIZE);
        int bytesReceived = recv(clientSocket, buff, BUFF_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            printf("Client disconnected.\n");
            handleClientDisconnect(clientSocket);
            return 0;
        }

        printf("Received from %d:\n%s\n", clientSocket, buff);

        int headerEndIndex = strcspn(buff, "\n");
        buff[headerEndIndex] = '\0';

        char *token = strtok(buff, " ");
        strncpy(messageType, token, 6);

        blockType = strtok(NULL, " ");

//        printf("messageType = %s\n", messageType);
//        printf("blockType = %s\n", blockType);
        if (buff[headerEndIndex + 1] != '\0')
            strcat(payload, buff + headerEndIndex + 1);
//        printf("payload = %s\n", payload);
    } while (strcmp(blockType, "LAST") != 0);

    strcat(payload, "\0");

//    printf("messageType = %s\n", messageType);
//    printf("payload = %s\n", payload);
    return 1;
}

void *handleRequest(void *arg) {
    pthread_detach(pthread_self());

    int clientSocket = *(int *) arg;
    ClientInfo *thisClient;
    ClientInfo *opponentClient;
    char messageType[10], payload[16 * BUFF_SIZE];
    char buff[BUFF_SIZE];

    while (handleReceive(clientSocket, messageType, payload)) {
//        printf("messageType = %s\n", messageType);
//        printf("payload = %s\n", payload);

        if (strcmp(messageType, "REGIST") == 0) {
            int res = handleCreateAccount(payload);
            switch (res) {
                case 0:
                    handleSend(clientSocket, "OK", "Account created successfully");
                    break;
                case 1:
                    handleSend(clientSocket, "ERROR", "Username already used");
                    break;
            }
        } else if (strcmp(messageType, "LOGIN") == 0) {
            Account *account = handleSignIn(payload);
            if (account == NULL) {
                handleSend(clientSocket, "ERROR", "Wrong username or password");
            } else {
                handleSend(clientSocket, "OK", "Signed in successfully");

                thisClient = new ClientInfo(clientSocket, account);
                clients.insert(thisClient);
                notifyOnlineStatusChange();
            }
        } else if (strcmp(messageType, "LSTONL") == 0) {
            payload[0] = '\0';
            char content[BUFF_SIZE];

            for (const auto &client: clients) {
                char status[20];
                if (client->status == 0) {
                    strcpy(status, "Available");
                } else {
                    strcpy(status, "In game");
                }

                sprintf(content, "%s %s\n", client->account->username.c_str(), status);
                strcat(payload, content);
            }

            handleSend(clientSocket, "LSTONL", payload);
        } else if (strcmp(messageType, "INVITE") == 0) {
            char *opponent = strtok(payload, "\n");
            opponentClient = findClientByUsername(opponent);

            handleSend(opponentClient->socket, "INVITE", (thisClient->account->username + "\n").c_str());
        } else if (strcmp(messageType, "INVRES") == 0) {
            char *opponent = strtok(payload, "\n");
            char *reply = strtok(NULL, "\n");
            opponentClient = findClientByUsername(opponent);

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%s\n%s\n", thisClient->account->username.c_str(), reply);
            handleSend(opponentClient->socket, "INVRES", buff);

            if (strcmp(reply, "ACCEPT") == 0) {
                printf("Establish game between %s and %s\n", thisClient->account->username.c_str(), opponent);

                thisClient->status = 1;
                opponentClient->status = 1;
                notifyOnlineStatusChange();

                GoGame *game = new GoGame(13);
                games[generateKey(clientSocket, opponentClient->socket)] = game;

                srand(time(NULL));
                int randomColor = rand() % 2 + 1;

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n", randomColor);
                handleSend(clientSocket, "SETUP", buff);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n", 3 - randomColor);
                handleSend(opponentClient->socket, "SETUP", buff);
            } else {
                opponentClient = NULL;
            }
        } else if (strcmp(messageType, "MOVE") == 0) {
            GoGame *game = games[generateKey(clientSocket, opponentClient->socket)];

            int color = atoi(strtok(payload, "\n"));
            char *coords = strtok(NULL, "\n");

            if (strcmp(coords, "PA") != 0) {
                if (game->play(coords, color)) {
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\n%s\n", color, coords);

                    vector <string> captured = game->getCaptured();
                    if (captured.size() > 0) {
                        for (string cap: captured) {
                            strcat(buff, (cap + " ").c_str());
                        }
                        strcat(buff, "\n");
                    }

                    handleSend(clientSocket, "MOVE", buff);
                    handleSend(opponentClient->socket, "MOVE", buff);
                } else {
                    handleSend(clientSocket, "MOVERR", "");
                }
            } else {
                if (game->pass() == 2) {
                    pair<float, float> scores = game->calculateScore();
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%.1f %.1f\n", scores.first, scores.second);
                    handleSend(clientSocket, "RESULT", buff);
                    handleSend(opponentClient->socket, "RESULT", buff);

                    thisClient->status = 0;
                    opponentClient->status = 0;
                    notifyOnlineStatusChange();
                } else {
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\n%s\n", color, coords);
                    handleSend(opponentClient->socket, "MOVE", buff);
                }
            }
        }
    }

    return NULL;
}

#endif //GAME_OF_GO_SERVER_CONTROLLER_HPP
