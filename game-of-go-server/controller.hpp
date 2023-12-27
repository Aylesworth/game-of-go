//
// Created by Admin on 11/21/2023.
//

#ifndef GAME_OF_GO_SERVER_CONTROLLER_HPP
#define GAME_OF_GO_SERVER_CONTROLLER_HPP

#include "service.hpp"
#include "go_engine.hpp"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <set>
#include <map>

using namespace std;

#define BUFF_SIZE 2048

int handleSend(int, const char *, const char *);

int handleReceive(int, char *, char *);

struct ClientInfo {
    int socket;
    Account *account;
    int status;

    ClientInfo() {}

    ClientInfo(int socket, Account *account, int status) :
            socket(socket), account(account), status(status) {}

    bool operator<(const ClientInfo &other) const {
        return account->username < other.account->username;
    }
};

set<ClientInfo *> clients;
Account *cpu = findAccount("@CPU");

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

int handleSend(int clientSocket, const char *messageType, const char *payload) {
    char buff[BUFF_SIZE];
    int bytesSent;
    int blockTypeSize = 1;
    int payloadlenSize = 4;
    int headerSize = strlen(messageType) + blockTypeSize + payloadlenSize + 3;

    printf("payload_length = %ld\n", strlen(payload));
    while (headerSize + strlen(payload) > BUFF_SIZE - 1) {
        printf("payload_length = %ld\n", strlen(payload));
        memset(buff, 0, BUFF_SIZE);
        printf("payload_length = %ld\n", strlen(payload));
        char payloadSubstring[BUFF_SIZE - headerSize];
        printf("payload_length = %ld\n", strlen(payload));
        strncpy(payloadSubstring, payload, BUFF_SIZE - 1 - headerSize);
        printf("payload_length = %ld\n", strlen(payload));
        sprintf(buff, "%s %s %ld\n%s", messageType, "M", strlen(payloadSubstring), payloadSubstring);
        printf("payload_length = %ld\n", strlen(payload));
        printf("msg = %s\n", buff);
        printf("payload_length = %ld\n", strlen(payload));

        bytesSent = send(clientSocket, buff, strlen(buff), 0);
        printf("payload_length = %ld\n", strlen(payload));
        if (bytesSent < 0) {
            return 0;
        }
        printf("payload_length = %ld\n", strlen(payload));

        payload = payload + (BUFF_SIZE - 1 - headerSize);
        printf("payload_length = %ld\n", strlen(payload));
    }

    memset(buff, 0, BUFF_SIZE);
    sprintf(buff, "%s %s %ld\n%s", messageType, "L", strlen(payload), payload);
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
            printf("Socket %d closed.\n", clientSocket);
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
    } while (strcmp(blockType, "L") != 0);

    strcat(payload, "\0");

//    printf("messageType = %s\n", messageType);
//    printf("payload = %s\n", payload);
    return 1;
}

void getGameResult(char *buff, GoGame *game) {
    game->calculateScore();

    memset(buff, 0, BUFF_SIZE);
    sprintf(buff, "%.1f %.1f\n%s\n%s\n", game->getBlackScore(), game->getWhiteScore(),
            game->getBlackTerritory().c_str(), game->getWhiteTerritory().c_str());
}

string calculateRankType(int elo) {
    if (elo < 100) {
        return "Unranked";
    }

    if (elo < 2100) {
        int kyu = (2099 - elo) / 100 + 1;
        return to_string(kyu) + "K";
    }

    if (elo < 2700) {
        int dan = (elo - 2000) / 100;
        return to_string(dan) + "D";
    }

    if (elo < 2940) {
        int dan = (elo - 2700) / 30 + 1;
        return to_string(dan) + "P";
    }

    return "9P";
}

void updateRanking(GoGame *game, Account *p1, Account *p2) {
    double S1 = (p1->id == game->getBlackPlayerId() && game->getBlackScore() > game->getWhiteScore())
                || (p1->id == game->getWhitePlayerId() && game->getWhiteScore() > game->getBlackScore())
                ? 1 : 0;
    double SE1 = 1.0 / (1.0 + exp((p2->elo - p1->elo) / 110));
    double K1 = -53 / 1300 * p1->elo + 1561 / 13;
    p1->elo += K1 * (S1 - SE1);
    p1->rankType = calculateRankType(p1->elo);

    double S2 = 1 - S1;
    double SE2 = 1 - SE1;
    double K2 = -53 / 1300 * p2->elo + 1561 / 13;
    p2->elo += K2 * (S2 - SE2);
    p2->rankType = calculateRankType(p2->elo);

    handleUpdateRanking(*p1);
    handleUpdateRanking(*p2);
}

void sendComputerMove(ClientInfo *player, GoGame *game, int color) {
    char buff[BUFF_SIZE];
    string move = game->generateMove(color);

    memset(buff, 0, BUFF_SIZE);
    sprintf(buff, "%d\n%s\n", color, move.c_str());

    vector <string> captured = game->getCaptured();
    if (captured.size() > 0) {
        for (string cap: captured) {
            strcat(buff, (cap + " ").c_str());
        }
        strcat(buff, "\n");
    }

    handleSend(player->socket, "MOVE", buff);

    if (move == "PA") {
        if (game->pass(color) == 2) {
            getGameResult(buff, game);
            handleSend(player->socket, "RESULT", buff);
            handleSaveGame(game);
            updateRanking(game, player->account, cpu);

            for (ClientInfo *client: clients) {
                if (client->socket == player->socket) {
                    client->status = 0;
                    notifyOnlineStatusChange();
                }
            }
        }
    }
}

void generateMatches(int number) {
    for (int i = 0; i < number; i++) {
        vector < Account * > players = getRandomPlayers(2);

        int boardSizes[3] = {9, 13, 19};
        GoGame *game = new GoGame(boardSizes[rand() % 3]);
        game->setId(generateGameId());
        game->setBlackPlayerId(players[0]->id);
        game->setWhitePlayerId(players[1]->id);

        int color = 1;
        string move;
        while (1) {
            move = game->generateMove(color);
            if (move == "PA") {
                int p = game->pass(color);
                printf("Pass count = %d\n", p);
                if (p == 2) {
                    game->calculateScore();
                    handleSaveGame(game);
                    updateRanking(game, players[0], players[1]);
                    printf("%.1f %.1f\n", game->getBlackScore(), game->getWhiteScore());
                    break;
                }
            }
            color = 3 - color;
        }
    }
}

// Handle each message from a client
void *handleRequest(void *arg) {
    pthread_detach(pthread_self());

    int clientSocket = *(int *) arg;
    ClientInfo *thisClient;
    ClientInfo *opponentClient;
    char messageType[10], payload[16 * BUFF_SIZE];
    char buff[BUFF_SIZE];

    while (handleReceive(clientSocket, messageType, payload)) {

        // Register account
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
        }

            // Login account
        else if (strcmp(messageType, "LOGIN") == 0) {
            Account *account = handleSignIn(payload);
            if (account == NULL) {
                handleSend(clientSocket, "ERROR", "Wrong username or password");
            } else {
                handleSend(clientSocket, "OK", "Signed in successfully");

                thisClient = new ClientInfo(clientSocket, account, 0);
                clients.insert(thisClient);
                notifyOnlineStatusChange();
            }
        }

            // Get online player list
        else if (strcmp(messageType, "LSTONL") == 0) {
            payload[0] = '\0';
            char content[BUFF_SIZE];

            for (const auto &client: clients) {
                if (client->account->username == thisClient->account->username) continue;

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
        }

            // Invite other player
        else if (strcmp(messageType, "INVITE") == 0) {
            char *opponent = strtok(payload, "\n");
            int boardSize = atoi(strtok(NULL, "\n"));

            if (strcmp(opponent, "@CPU") == 0) {
                opponentClient = NULL;

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%s\n%d\n%s\n", "@CPU", boardSize, "ACCEPT");
                handleSend(clientSocket, "INVRES", buff);

                printf("Establish game between %s and %s\n", thisClient->account->username.c_str(), "@CPU");

                thisClient->status = 1;
                notifyOnlineStatusChange();

                GoGame *game = new GoGame(boardSize);
                game->setId(generateGameId());
                games[clientSocket] = game;

                srand(time(NULL));
                int randomColor = rand() % 2 + 1;

                game->setBlackPlayerId(randomColor == 1 ? thisClient->account->id : -1);
                game->setWhitePlayerId(randomColor == 1 ? -1 : thisClient->account->id);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n%d\n", boardSize, randomColor);
                handleSend(clientSocket, "SETUP", buff);

                if (randomColor != 1) {
                    usleep(100000);
                    sendComputerMove(thisClient, game, 3 - randomColor);
                }

                continue;
            }

            opponentClient = findClientByUsername(opponent);

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%s\n%d\n", thisClient->account->username.c_str(), boardSize);
            handleSend(opponentClient->socket, "INVITE", buff);

        }

            // Reply to invitation
        else if (strcmp(messageType, "INVRES") == 0) {
            char *opponent = strtok(payload, "\n");
            int boardSize = atoi(strtok(NULL, "\n"));
            char *reply = strtok(NULL, "\n");
            opponentClient = findClientByUsername(opponent);

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%s\n%d\n%s\n", thisClient->account->username.c_str(), boardSize, reply);
            handleSend(opponentClient->socket, "INVRES", buff);

            if (strcmp(reply, "ACCEPT") == 0) {
                printf("Establish game between %s and %s\n", thisClient->account->username.c_str(), opponent);

                thisClient->status = 1;
                opponentClient->status = 1;
                notifyOnlineStatusChange();

                GoGame *game = new GoGame(boardSize);
                game->setId(generateGameId());

                games[clientSocket] = game;
                games[opponentClient->socket] = game;

                srand(time(NULL));
                int randomColor = rand() % 2 + 1;

                game->setBlackPlayerId(randomColor == 1 ? thisClient->account->id : opponentClient->account->id);
                game->setWhitePlayerId(randomColor == 1 ? opponentClient->account->id : thisClient->account->id);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n%d\n", boardSize, randomColor);
                handleSend(clientSocket, "SETUP", buff);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n%d\n", boardSize, 3 - randomColor);
                handleSend(opponentClient->socket, "SETUP", buff);
            } else {
                opponentClient = NULL;
            }
        }

            // Make a move
        else if (strcmp(messageType, "MOVE") == 0) {
            GoGame *game = games[clientSocket];

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
                    if (opponentClient != NULL) {
                        usleep(10000);
                        handleSend(opponentClient->socket, "MOVE", buff);
                    } else {
                        sendComputerMove(thisClient, game, 3 - color);
                    }
                } else {
                    handleSend(clientSocket, "MOVERR", "");
                }
            } else {
                if (game->pass(color) == 2) {
                    getGameResult(buff, game);
                    handleSaveGame(game);

                    handleSend(clientSocket, "RESULT", buff);
                    if (opponentClient != NULL) {
                        handleSend(opponentClient->socket, "RESULT", buff);
                        updateRanking(game, thisClient->account, opponentClient->account);
                    } else {
                        updateRanking(game, thisClient->account, cpu);
                    }
                } else {
                    if (opponentClient == NULL) {
                        game->calculateScore();
                        float blackScore = game->getBlackScore();
                        float whiteScore = game->getWhiteScore();

                        if ((color == 1 && blackScore < whiteScore) ||
                            (color == 2 && blackScore > whiteScore)) {
                            game->pass(3 - color);

                            memset(buff, 0, BUFF_SIZE);
                            sprintf(buff, "%d\n%s\n", 3 - color, "PA");
                            handleSend(clientSocket, "MOVE", buff);

                            getGameResult(buff, game);
                            handleSend(clientSocket, "RESULT", buff);
                            handleSaveGame(game);
                            updateRanking(game, thisClient->account, cpu);
                            continue;
                        }
                        sendComputerMove(thisClient, game, 3 - color);
                        continue;
                    }
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\n%s\n", color, coords);
                    handleSend(opponentClient->socket, "MOVE", buff);
                }
            }
        }

            // Request game interrupt
        else if (strcmp(messageType, "INTRPT") == 0) {
            GoGame *game = games[clientSocket];

            int color = atoi(strtok(payload, "\n"));
            char *interruptType = strtok(NULL, "\n");

            if (strcmp(interruptType, "RESIGN") == 0) {
                if (opponentClient != NULL) {
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\nRESIGN\n", color);
                    handleSend(opponentClient->socket, "INTRPT", buff);
                }

                game->resign(color);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%.1f %.1f\n%s\n%s\n", game->getBlackScore(), game->getWhiteScore(),
                        game->getBlackTerritory().c_str(), game->getWhiteTerritory().c_str());

                usleep(10000);
                handleSend(clientSocket, "RESULT", buff);
                if (opponentClient != NULL) {
                    handleSend(opponentClient->socket, "RESULT", buff);
                    updateRanking(game, thisClient->account, opponentClient->account);
                } else {
                    updateRanking(game, thisClient->account, cpu);
                }

                handleSaveGame(game);
            }
        }

            // Confirm game result
        else if (strcmp(messageType, "RESACK") == 0) {
            thisClient->status = 0;
            opponentClient = NULL;
            games[clientSocket] = NULL;
            notifyOnlineStatusChange();
        }

            // Get history of played games
        else if (strcmp(messageType, "HISTRY") == 0) {
            vector < GameRecord * > games = handleGetHistory(thisClient->account->id);
            memset(payload, 0, 16 * BUFF_SIZE);
            memset(buff, 0, BUFF_SIZE);
            for (const GameRecord *game: games) {
                sprintf(buff, "%s %d %d %s %.2f %.2f %ld\n", game->id.c_str(), game->boardSize,
                        game->color, game->opponent.c_str(),
                        game->blackScore, game->whiteScore, game->time);
                strcat(payload, buff);
            }
            strcat(payload, "\0");
            handleSend(clientSocket, "HISTRY", payload);
        }

            // Get game replay
        else if (strcmp(messageType, "REPLAY") == 0) {
            char *id = strtok(payload, "\n");
            printf("id = %s\n", id);
            GameReplay *replay = handleGetReplay(id);
            printf("replay = %p\n", replay);
            const char *data = (string(payload) + "\n" + replay->log + "\n" + replay->blackTerritory + " \n" +
                                replay->whiteTerritory +
                                " \n").c_str();
            printf("data = %p\n", data);
            handleSend(clientSocket, "REPLAY", data);
            printf("data = %p\n", data);
        }
    }

    close(clientSocket);
    return NULL;
}

#endif //GAME_OF_GO_SERVER_CONTROLLER_HPP
