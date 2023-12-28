//
// Created by Admin on 11/21/2023.
//

#ifndef GAME_OF_GO_SERVER_CONTROLLER_HPP
#define GAME_OF_GO_SERVER_CONTROLLER_HPP

#include "dao.hpp"
#include "go_engine.hpp"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <openssl/md5.h>
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

struct Context {
    ClientInfo *selfInfo;
    ClientInfo *opponentInfo;
    GoGame *game;
};

map<int, Context> ctx;

//map<int, GoGame *>
//        games;

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

void handleClientDisconnect(int sock) {
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if ((*it)->socket == sock) {
            clients.erase(it);
            notifyOnlineStatusChange();
            return;
        }
    }

    ctx[sock].selfInfo = NULL;
    ctx[sock].opponentInfo = NULL;
    ctx[sock].game = NULL;
}

int handleSend(int sock, const char *messageType, const char *payload) {
    char buff[BUFF_SIZE];
    int bytesSent;
    int blockTypeSize = 1;
    int payloadlenSize = 4;
    int headerSize = strlen(messageType) + blockTypeSize + payloadlenSize + 3;

    while (headerSize + strlen(payload) > BUFF_SIZE - 1) {
        memset(buff, 0, BUFF_SIZE);
        char payloadSubstring[BUFF_SIZE - headerSize];
        strncpy(payloadSubstring, payload, BUFF_SIZE - 1 - headerSize);
        sprintf(buff, "%s %s %ld\n%s", messageType, "M", strlen(payloadSubstring), payloadSubstring);

        bytesSent = send(sock, buff, strlen(buff), 0);
        if (bytesSent < 0) {
            return 0;
        }

        payload = payload + (BUFF_SIZE - 1 - headerSize);
    }

    memset(buff, 0, BUFF_SIZE);
    sprintf(buff, "%s %s %ld\n%s", messageType, "L", strlen(payload), payload);
    bytesSent = send(sock, buff, strlen(buff), 0);
    printf("Sent to %d:\n%s\n", sock, buff);

    return 1;
}

int handleReceive(int sock, char *messageType, char *payload) {
    char buff[BUFF_SIZE];
    char *blockType;
    payload[0] = '\0';

    memset(payload, 0, 16 * BUFF_SIZE);
    do {
        memset(buff, 0, BUFF_SIZE);
        int bytesReceived = recv(sock, buff, BUFF_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            printf("Socket %d closed.\n", sock);
            handleClientDisconnect(sock);
            return 0;
        }

        printf("Received from %d:\n%s\n", sock, buff);

        int headerEndIndex = strcspn(buff, "\n");
        buff[headerEndIndex] = '\0';

        char *token = strtok(buff, " ");
        strncpy(messageType, token, 6);

        blockType = strtok(NULL, " ");

        if (buff[headerEndIndex + 1] != '\0')
            strcat(payload, buff + headerEndIndex + 1);
    } while (strcmp(blockType, "L") != 0);

    strcat(payload, "\0");
    return 1;
}

char *encodeMD5(const char *input) {
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    MD5_Update(&md5Context, input, strlen(input));

    unsigned char md5_result[MD5_DIGEST_LENGTH];
    MD5_Final(md5_result, &md5Context);

    char *md5_string = (char *) malloc(2 * MD5_DIGEST_LENGTH + 1);

    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&md5_string[i * 2], "%02x", md5_result[i]);
    }

    md5_string[2 * MD5_DIGEST_LENGTH] = '\0';

    return md5_string;
}

string generateGameId() {
    string id;
    string pool = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                  "abcdefghijklmnopqrstuvwxyz"
                  "0123456789";
    do {
        id = "";
        srand(time(NULL));
        for (int i = 0; i < 16; i++) {
            id += pool[rand() % pool.size()];
        }
    } while (doesGameIdExist(id));

    printf("Generate new id: %s\n", id.c_str());

    return id;
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

void updatePlayerRankings(GoGame *game, Account *p1, Account *p2) {
    double S1 = (p1->id == game->getBlackPlayerId() && game->getBlackScore() > game->getWhiteScore())
                || (p1->id == game->getWhitePlayerId() && game->getWhiteScore() > game->getBlackScore())
                ? 1 : 0;
    double SE1 = 1.0 / (1.0 + exp((p2->elo - p1->elo) / 110));
    double K1 = -53.0 / 1300 * p1->elo + 1561.0 / 13;
    int elo1_old = p1->elo;
    p1->elo = (int) round(p1->elo + K1 * (S1 - SE1));
    p1->rankType = calculateRankType(p1->elo);

    double S2 = 1 - S1;
    double SE2 = 1 - SE1;
    double K2 = -53.0 / 1300 * p2->elo + 1561.0 / 13;
    int elo2_old = p2->elo;
    p2->elo = (int) round(p2->elo + K2 * (S2 - SE2));
    p2->rankType = calculateRankType(p2->elo);

    printf("R1 = %d + %.2f (%.2f - %.2f) = %d\n", elo1_old, K1, S1, SE1, p1->elo);
    printf("R2 = %d + %.2f (%.2f - %.2f) = %d\n", elo2_old, K2, S2, SE2, p2->elo);
    updateRanking(*p1);
    updateRanking(*p2);
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
            saveGame(game);
            updatePlayerRankings(game, player->account, cpu);

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
                if (p == 2) {
                    game->calculateScore();
                    saveGame(game);
                    updatePlayerRankings(game, players[0], players[1]);
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

    int sock = *(int *) arg;
//    ClientInfo *ctx[sock].selfInfo;
//    ClientInfo *ctx[sock].opponentInfo;
    char messageType[10], payload[16 * BUFF_SIZE];
    char buff[BUFF_SIZE];

    while (handleReceive(sock, messageType, payload)) {

        // Register account
        if (strcmp(messageType, "REGIST") == 0) {
            char *username = strtok(payload, "\n");
            char *password = strtok(NULL, "\n");

            if (findAccount(username) != NULL) {
                handleSend(sock, "ERROR", "Username already used");
                continue;
            }

            Account account;
            account.username = username;
            account.password = encodeMD5(password);

            createAccount(account);
            handleSend(sock, "OK", "Account created successfully");
            continue;
        }

        // Login account
        if (strcmp(messageType, "LOGIN") == 0) {
            char *username = strtok(payload, "\n");
            char *password = strtok(NULL, "\n");

            Account *account = findAccount(username);
            if (account == NULL) {
                handleSend(sock, "ERROR", "Wrong username or password");
                continue;
            }

            string passwordHash = encodeMD5(password);
            if (passwordHash != account->password) {
                handleSend(sock, "ERROR", "Wrong username or password");
                continue;
            }

            account->password = "";

            handleSend(sock, "OK", "Signed in successfully");

            ClientInfo *info = new ClientInfo(sock, account, 0);
            ctx[sock].selfInfo = info;
            clients.insert(info);
            notifyOnlineStatusChange();
            continue;
        }

        // Get online player list
        if (strcmp(messageType, "LSTONL") == 0) {
            payload[0] = '\0';
            char content[BUFF_SIZE];

            for (const auto &client: clients) {
                if (client->account->username == ctx[sock].selfInfo->account->username) continue;

                char status[20];
                if (client->status == 0) {
                    strcpy(status, "Available");
                } else {
                    strcpy(status, "In game");
                }

                sprintf(content, "%s %s\n", client->account->username.c_str(), status);
                strcat(payload, content);
            }

            handleSend(sock, "LSTONL", payload);
            continue;
        }

        // Invite other player
        if (strcmp(messageType, "INVITE") == 0) {
            char *opponent = strtok(payload, "\n");
            int boardSize = atoi(strtok(NULL, "\n"));

            if (strcmp(opponent, "@CPU") == 0) {
                ctx[sock].opponentInfo = NULL;

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%s\n%d\n%s\n", "@CPU", boardSize, "ACCEPT");
                handleSend(sock, "INVRES", buff);

                printf("Establish game between %s and %s\n", ctx[sock].selfInfo->account->username.c_str(), "@CPU");

                ctx[sock].selfInfo->status = 1;
                notifyOnlineStatusChange();

                GoGame *game = new GoGame(boardSize);
                game->setId(generateGameId());
                ctx[sock].game = game;

                srand(time(NULL));
                int randomColor = rand() % 2 + 1;

                game->setBlackPlayerId(randomColor == 1 ? ctx[sock].selfInfo->account->id : -1);
                game->setWhitePlayerId(randomColor == 1 ? -1 : ctx[sock].selfInfo->account->id);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n%d\n", boardSize, randomColor);
                handleSend(sock, "SETUP", buff);

                if (randomColor != 1) {
                    usleep(100000);
                    sendComputerMove(ctx[sock].selfInfo, game, 3 - randomColor);
                }

                continue;
            }

            ctx[sock].opponentInfo = findClientByUsername(opponent);

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%s\n%d\n", ctx[sock].selfInfo->account->username.c_str(), boardSize);
            handleSend(ctx[sock].opponentInfo->socket, "INVITE", buff);
            continue;
        }

        // Reply to invitation
        if (strcmp(messageType, "INVRES") == 0) {
            char *opponent = strtok(payload, "\n");
            int boardSize = atoi(strtok(NULL, "\n"));
            char *reply = strtok(NULL, "\n");
            ctx[sock].opponentInfo = findClientByUsername(opponent);

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%s\n%d\n%s\n", ctx[sock].selfInfo->account->username.c_str(), boardSize, reply);
            handleSend(ctx[sock].opponentInfo->socket, "INVRES", buff);

            if (strcmp(reply, "ACCEPT") == 0) {
                printf("Establish game between %s and %s\n", ctx[sock].selfInfo->account->username.c_str(), opponent);

                ctx[sock].selfInfo->status = 1;
                ctx[sock].opponentInfo->status = 1;
                notifyOnlineStatusChange();

                GoGame *game = new GoGame(boardSize);
                game->setId(generateGameId());

                ctx[sock].game = game;
                ctx[ctx[sock].opponentInfo->socket].game = game;

                srand(time(NULL));
                int randomColor = rand() % 2 + 1;

                game->setBlackPlayerId(randomColor == 1 ? ctx[sock].selfInfo->account->id : ctx[sock].opponentInfo->account->id);
                game->setWhitePlayerId(randomColor == 1 ? ctx[sock].opponentInfo->account->id : ctx[sock].selfInfo->account->id);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n%d\n", boardSize, randomColor);
                handleSend(sock, "SETUP", buff);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%d\n%d\n", boardSize, 3 - randomColor);
                handleSend(ctx[sock].opponentInfo->socket, "SETUP", buff);
            } else {
                ctx[sock].opponentInfo = NULL;
            }
            continue;
        }

        // Make a move
        if (strcmp(messageType, "MOVE") == 0) {
            GoGame *game = ctx[sock].game;

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

                    handleSend(sock, "MOVE", buff);
                    if (ctx[sock].opponentInfo != NULL) {
                        usleep(10000);
                        handleSend(ctx[sock].opponentInfo->socket, "MOVE", buff);
                    } else {
                        sendComputerMove(ctx[sock].selfInfo, game, 3 - color);
                    }
                } else {
                    handleSend(sock, "MOVERR", "");
                }
            } else {
                if (game->pass(color) == 2) {
                    getGameResult(buff, game);
                    saveGame(game);

                    handleSend(sock, "RESULT", buff);
                    if (ctx[sock].opponentInfo != NULL) {
                        handleSend(ctx[sock].opponentInfo->socket, "RESULT", buff);
                        updatePlayerRankings(game, ctx[sock].selfInfo->account, ctx[sock].opponentInfo->account);
                    } else {
                        updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                    }
                } else {
                    if (ctx[sock].opponentInfo == NULL) {
                        game->calculateScore();
                        float blackScore = game->getBlackScore();
                        float whiteScore = game->getWhiteScore();

                        if ((color == 1 && blackScore < whiteScore) ||
                            (color == 2 && blackScore > whiteScore)) {
                            game->pass(3 - color);

                            memset(buff, 0, BUFF_SIZE);
                            sprintf(buff, "%d\n%s\n", 3 - color, "PA");
                            handleSend(sock, "MOVE", buff);

                            getGameResult(buff, game);
                            handleSend(sock, "RESULT", buff);
                            saveGame(game);
                            updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                            continue;
                        }
                        sendComputerMove(ctx[sock].selfInfo, game, 3 - color);
                        continue;
                    }
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\n%s\n", color, coords);
                    handleSend(ctx[sock].opponentInfo->socket, "MOVE", buff);
                }
            }
            continue;
        }

        // Request game interrupt
        if (strcmp(messageType, "INTRPT") == 0) {
            GoGame *game = ctx[sock].game;

            int color = atoi(strtok(payload, "\n"));
            char *interruptType = strtok(NULL, "\n");

            if (strcmp(interruptType, "RESIGN") == 0) {
                if (ctx[sock].opponentInfo != NULL) {
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\nRESIGN\n", color);
                    handleSend(ctx[sock].opponentInfo->socket, "INTRPT", buff);
                }

                game->resign(color);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%.1f %.1f\n%s\n%s\n", game->getBlackScore(), game->getWhiteScore(),
                        game->getBlackTerritory().c_str(), game->getWhiteTerritory().c_str());

                usleep(10000);
                handleSend(sock, "RESULT", buff);
                if (ctx[sock].opponentInfo != NULL) {
                    handleSend(ctx[sock].opponentInfo->socket, "RESULT", buff);
                    updatePlayerRankings(game, ctx[sock].selfInfo->account, ctx[sock].opponentInfo->account);
                } else {
                    updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                }

                saveGame(game);
            }

            continue;
        }

        // Confirm game result
        if (strcmp(messageType, "RESACK") == 0) {
            ctx[sock].selfInfo->status = 0;
            ctx[sock].opponentInfo = NULL;
            ctx[sock].game = NULL;
            notifyOnlineStatusChange();
            continue;
        }

        // Get history of played games
        if (strcmp(messageType, "HISTRY") == 0) {
            vector < GameRecord * > games = findGamesByPlayer(ctx[sock].selfInfo->account->id);
            memset(payload, 0, 16 * BUFF_SIZE);
            memset(buff, 0, BUFF_SIZE);
            for (const GameRecord *game: games) {
                sprintf(buff, "%s %d %d %s %.2f %.2f %ld\n", game->id.c_str(), game->boardSize,
                        game->color, game->opponent.c_str(),
                        game->blackScore, game->whiteScore, game->time);
                strcat(payload, buff);
            }
            strcat(payload, "\0");
            handleSend(sock, "HISTRY", payload);
            continue;
        }

        // Get game replay
        if (strcmp(messageType, "REPLAY") == 0) {
            char *id = strtok(payload, "\n");
            printf("id = %s\n", id);
            GameReplay *replay = getGameReplayInfo(id);
            printf("replay = %p\n", replay);
            const char *data = (string(payload) + "\n" + replay->log + "\n" + replay->blackTerritory + " \n" +
                                replay->whiteTerritory +
                                " \n").c_str();
            printf("data = %p\n", data);
            handleSend(sock, "REPLAY", data);
            printf("data = %p\n", data);
            continue;
        }

        // Get rankings
        if (strcmp(messageType, "RANKIN") == 0) {
            vector <pair<int, Account *>> rankings = getRankings();
            string data = "";
            for (auto r: rankings) {
                data += to_string(r.first) + " " +
                        r.second->username + " " +
                        to_string(r.second->elo) + " " +
                        r.second->rankType + "\n";
            }
            handleSend(sock, "RANKIN", data.c_str());
            continue;
        }
    }

    close(sock);
    return NULL;
}

#endif //GAME_OF_GO_SERVER_CONTROLLER_HPP
