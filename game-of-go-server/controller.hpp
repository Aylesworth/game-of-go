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

int sendMessage(int, const char *, const char *);

int receiveMessage(int, char *, char *);

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
    vector<int> challengers;
    GoGame *game;
};

map<int, Context> ctx;

struct MatchingInfo {
    ClientInfo *client;
    int boardSize;

    MatchingInfo(ClientInfo *client, int boardSize) :
            client(client), boardSize(boardSize) {}
};

vector <MatchingInfo> matching;

pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ctxMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t matchingMutex = PTHREAD_MUTEX_INITIALIZER;

ClientInfo *findClientByUsername(string username) {
    pthread_mutex_lock(&clientsMutex);
    for (const auto &client: clients) {
        if (client->account->username == username) {
            pthread_mutex_unlock(&clientsMutex);
            return client;
        }
    }
    pthread_mutex_unlock(&clientsMutex);
    return NULL;
}

void setInGameStatus(int sock) {
    pthread_mutex_lock(&ctxMutex);
    ctx[sock].selfInfo->status = 1;
    for (auto &entry: ctx) {
        Context &c = entry.second;
        auto it = find(c.challengers.begin(), c.challengers.end(), sock);
        if (it != c.challengers.end())
            c.challengers.erase(it);
    }
    pthread_mutex_unlock(&ctxMutex);
}

void notifyOnlineStatusChange() {
    for (const auto &client: clients) {
        if (client->status == 0) {
            sendMessage(client->socket, "CHGONL", "");
        }
    }
}

void handleClientDisconnect(int sock) {
    pthread_mutex_lock(&clientsMutex);
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if ((*it)->socket == sock) {
            clients.erase(it);
            notifyOnlineStatusChange();
            break;
        }
    }
    pthread_mutex_unlock(&clientsMutex);

    pthread_mutex_lock(&ctxMutex);
    ctx[sock].selfInfo = NULL;
    ctx[sock].opponentInfo = NULL;
    ctx[sock].game = NULL;
    ctx[sock].challengers.clear();
    pthread_mutex_unlock(&ctxMutex);
}

int sendMessage(int sock, const char *messageType, const char *payload) {
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

int receiveMessage(int sock, char *messageType, char *payload) {
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

int calculateEloChange(double result, int selfElo, int opponentElo) {
    double winningChance = 1.0 / (1.0 + exp((opponentElo - selfElo) / 110.0));
    double K = 1561.0 / 13 - 53.0 / 1300 * selfElo;
    if (K < 10) K = 10;
    int change = (int) round(K * (result - winningChance));
    printf("new elo: %d + %.1f * (%.1f - %.3f) = %d\n", selfElo, K, result, winningChance, selfElo + change);
    return change;
}

void updatePlayerRankings(GoGame *game, Account *p1, Account *p2) {
    if (!game->isRanked()) return;

    double result1 = (p1->id == game->getBlackPlayerId() && game->getBlackScore() > game->getWhiteScore())
                     || (p1->id == game->getWhitePlayerId() && game->getWhiteScore() > game->getBlackScore())
                     ? 1 : (game->getBlackScore() == game->getWhiteScore() ? 0.5 : 0);
    double result2 = 1 - result1;

    int change1 = calculateEloChange(result1, p1->elo, p2->elo);
    int change2 = calculateEloChange(result2, p2->elo, p1->elo);

    p1->elo += change1;
    p1->rankType = calculateRankType(p1->elo);

    p2->elo += change2;
    p2->rankType = calculateRankType(p2->elo);

    game->setBlackEloChange(p1->id == game->getBlackPlayerId() ? change1 : change2);
    game->setWhiteEloChange(p1->id == game->getWhitePlayerId() ? change1 : change2);
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

    sendMessage(player->socket, "MOVE", buff);

    if (move == "PA") {
        if (game->pass(color) == 2) {
            getGameResult(buff, game);
            sendMessage(player->socket, "RESULT", buff);
            updatePlayerRankings(game, player->account, cpu);
            saveGame(game);

            pthread_mutex_lock(&clientsMutex);
            for (ClientInfo *client: clients) {
                if (client->socket == player->socket) {
                    client->status = 0;
                    notifyOnlineStatusChange();
                }
            }
            pthread_mutex_unlock(&clientsMutex);
            return;
        }
    }
}

void setupGame(int sock1, int sock2, GoGame *game) {
    char buff[BUFF_SIZE];
    if (sock1 > 0 && sock2 > 0) {
        pthread_mutex_lock(&ctxMutex);
        ClientInfo *player1 = ctx[sock1].selfInfo;
        ClientInfo *player2 = ctx[sock2].selfInfo;

        printf("Establish game between %s and %s\n",
               player1->account->username.c_str(),
               player2->account->username.c_str());

        ctx[sock1].opponentInfo = player2;
        ctx[sock2].opponentInfo = player1;
        setInGameStatus(sock1);
        setInGameStatus(sock2);
        notifyOnlineStatusChange();

        game->setId(generateGameId());

        ctx[sock1].game = game;
        ctx[sock2].game = game;
        pthread_mutex_unlock(&ctxMutex);

        srand(time(NULL));
        int randomColor = rand() % 2 + 1;

        game->setBlackPlayerId(
                randomColor == 1 ? player1->account->id : player2->account->id);
        game->setWhitePlayerId(
                randomColor == 1 ? player2->account->id : player1->account->id);

        memset(buff, 0, BUFF_SIZE);
        sprintf(buff, "%s (ELO %d)\n%d\n%d\n%.2f\n%d %d %d %d\n",
                player2->account->username.c_str(),
                player2->account->elo,
                game->getBoardSize(), randomColor,
                game->getKomi(),
                game->getTimeSystem(),
                game->getMainTimeSecondsLeft(0),
                game->getByoyomiTimeSeconds(),
                game->getByoyomiPeriodsLeft(0));
        sendMessage(sock1, "SETUP", buff);

        memset(buff, 0, BUFF_SIZE);
        sprintf(buff, "%s (ELO %d)\n%d\n%d\n%.2f\n%d %d %d %d\n",
                player1->account->username.c_str(),
                player1->account->elo,
                game->getBoardSize(), 3 - randomColor,
                game->getKomi(),
                game->getTimeSystem(),
                game->getMainTimeSecondsLeft(0),
                game->getByoyomiTimeSeconds(),
                game->getByoyomiPeriodsLeft(0));
        sendMessage(sock2, "SETUP", buff);

        if (game->getTimeSystem() == 1) {
            usleep(10000);
            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "1\nM\n%d\n", game->getMainTimeSecondsLeft(1));
            sendMessage(sock1, "BYOYOM", buff);
            sendMessage(sock2, "BYOYOM", buff);
        }
    } else {
        int sock;
        if (sock1 == -1) sock = sock2;
        else sock = sock1;

        pthread_mutex_lock(&ctxMutex);
        printf("Establish game between %s and %s\n", ctx[sock].selfInfo->account->username.c_str(), "@CPU");

        setInGameStatus(sock);
        notifyOnlineStatusChange();

        game->setId(generateGameId());
        ctx[sock].game = game;

        srand(time(NULL));
        int randomColor = rand() % 2 + 1;

        game->setBlackPlayerId(randomColor == 1 ? ctx[sock].selfInfo->account->id : -1);
        game->setWhitePlayerId(randomColor == 1 ? -1 : ctx[sock].selfInfo->account->id);

        memset(buff, 0, BUFF_SIZE);
        sprintf(buff, "CPU (ELO %d)\n%d\n%d\n%.2f\n%d %d %d %d\n",
                cpu->elo,
                game->getBoardSize(),
                randomColor,
                game->getKomi(),
                game->getTimeSystem(),
                game->getMainTimeSecondsLeft(0),
                game->getByoyomiTimeSeconds(),
                game->getByoyomiPeriodsLeft(0));
        sendMessage(sock, "SETUP", buff);

        usleep(100000);

        if (randomColor != 1) {
            sendComputerMove(ctx[sock].selfInfo, game, 3 - randomColor);
        }
        pthread_mutex_unlock(&ctxMutex);
    }
}

void generateMatches(int number) {
    for (int i = 0; i < number; i++) {
        vector < Account * > players = getRandomPlayers(2);

        int boardSizes[3] = {9, 13, 19};
        GoGame *game = new GoGame(boardSizes[rand() % 3], 6.5, 0, -1, -1, -1, 1);
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
                    updatePlayerRankings(game, players[0], players[1]);
                    saveGame(game);
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
    char messageType[10], payload[16 * BUFF_SIZE];
    char buff[BUFF_SIZE];

    while (receiveMessage(sock, messageType, payload)) {
        // Register account
        if (strcmp(messageType, "REGIST") == 0) {
            char *username = strtok(payload, "\n");
            char *password = strtok(NULL, "\n");

            if (findAccount(username) != NULL) {
                sendMessage(sock, "ERROR", "Username already used");
                continue;
            }

            Account account;
            account.username = username;
            account.password = encodeMD5(password);

            createAccount(account);
            sendMessage(sock, "OK", "Account created successfully");
            continue;
        }

        // Login account
        if (strcmp(messageType, "LOGIN") == 0) {
            char *username = strtok(payload, "\n");
            char *password = strtok(NULL, "\n");

            ClientInfo *client = findClientByUsername(username);
            if (client != NULL) {
                sendMessage(sock, "ERROR", "Account is currently logged in");
                continue;
            }

            Account *account = findAccount(username);
            if (account == NULL) {
                sendMessage(sock, "ERROR", "Wrong username or password");
                continue;
            }

            string passwordHash = encodeMD5(password);
            if (passwordHash != account->password) {
                sendMessage(sock, "ERROR", "Wrong username or password");
                continue;
            }

            account->password = "";

            sendMessage(sock, "OK", "Signed in successfully");

            ClientInfo *info = new ClientInfo(sock, account, 0);
            pthread_mutex_lock(&ctxMutex);
            ctx[sock].selfInfo = info;
            pthread_mutex_unlock(&ctxMutex);
            pthread_mutex_lock(&clientsMutex);
            clients.insert(info);
            notifyOnlineStatusChange();
            pthread_mutex_unlock(&clientsMutex);
            continue;
        }

        // Get online player list
        if (strcmp(messageType, "LSTONL") == 0) {
            payload[0] = '\0';
            char content[BUFF_SIZE];

            pthread_mutex_lock(&clientsMutex);
            for (const auto &client: clients) {
                if (client->status != 0) continue;
                if (client->account->username == ctx[sock].selfInfo->account->username) continue;

                sprintf(content, "%s (ELO %d)\n", client->account->username.c_str(), client->account->elo);
                strcat(payload, content);
            }
            pthread_mutex_unlock(&clientsMutex);

            sendMessage(sock, "LSTONL", payload);
            continue;
        }

        // Auto-matching
        if (strcmp(messageType, "MATCH") == 0) {
            int boardSize = atoi(strtok(payload, "\n"));

            int matched = 0;
            pthread_mutex_lock(&matchingMutex);
            for (MatchingInfo m: matching) {
                if (sock != m.client->socket
                    && ctx[sock].selfInfo->account->rankType == m.client->account->rankType
                    && boardSize == m.boardSize) {
                    matched = 1;
                    for (auto it = matching.begin(); it != matching.end(); it++) {
                        if ((*it).client->socket == m.client->socket) {
                            matching.erase(it);
                            break;
                        }
                    }

                    setupGame(sock, m.client->socket, new GoGame(boardSize, 6.5, 0, -1, -1, -1, 1));
                    break;
                }
            }

            if (!matched) {
                matching.push_back(MatchingInfo(ctx[sock].selfInfo, boardSize));
            }
            pthread_mutex_unlock(&matchingMutex);
            continue;
        }

        // Cancel matching
        if (strcmp(messageType, "MATCCL") == 0) {
            pthread_mutex_lock(&matchingMutex);
            for (auto it = matching.begin(); it != matching.end(); it++) {
                if ((*it).client->socket == sock) {
                    matching.erase(it);
                    break;
                }
            }
            pthread_mutex_unlock(&matchingMutex);
            continue;
        }

        // Invite other player
        if (strcmp(messageType, "INVITE") == 0) {
            char *opponent = strtok(payload, "\n");
            char *params = payload + strlen(opponent) + 1;

            if (strcmp(opponent, "@CPU") == 0) {
                int boardSize = atoi(strtok(params, "\n"));
                float komi = strtof(strtok(NULL, "\n"), NULL);
                int timeSystem = atoi(strtok(NULL, " "));
                int mainTimeSeconds = atoi(strtok(NULL, " "));
                int byoyomiTimeSeconds = atoi(strtok(NULL, " "));
                int byoyomiPeriods = atoi(strtok(NULL, "\n"));
                bool ranked = atoi(strtok(NULL, "\n"));

                pthread_mutex_lock(&ctxMutex);
                ctx[sock].opponentInfo = NULL;
                pthread_mutex_unlock(&ctxMutex);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%s\n%s\n", "@CPU", "ACCEPT");
                sendMessage(sock, "INVRES", buff);

                GoGame *game = new GoGame(boardSize, komi, 0, -1, -1, -1, ranked);
                setupGame(sock, -1, game);
                continue;
            }

            pthread_mutex_lock(&ctxMutex);
            ctx[sock].opponentInfo = findClientByUsername(opponent);
            int oppsock = ctx[sock].opponentInfo->socket;
            ctx[oppsock].challengers.push_back(sock);

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%s\n%s\n", ctx[sock].selfInfo->account->username.c_str(), params);
            pthread_mutex_unlock(&ctxMutex);
            sendMessage(oppsock, "INVITE", buff);
            continue;
        }

        // Cancel invitation
        if (strcmp(messageType, "INVCCL") == 0) {
            char *opponent = strtok(payload, "\n");
            int oppsock = findClientByUsername(opponent)->socket;

            pthread_mutex_lock(&ctxMutex);
            auto it = find(ctx[oppsock].challengers.begin(), ctx[oppsock].challengers.end(), sock);
            if (it != ctx[oppsock].challengers.end()) {
                ctx[oppsock].challengers.erase(it);
            }
            pthread_mutex_unlock(&ctxMutex);

            continue;
        }

        // Reply to invitation
        if (strcmp(messageType, "INVRES") == 0) {
            char *opponent = strtok(payload, "\n");
            int boardSize = atoi(strtok(NULL, "\n"));
            float komi = strtof(strtok(NULL, "\n"), NULL);
            int timeSystem = atoi(strtok(NULL, " "));
            int mainTimeSeconds = atoi(strtok(NULL, " "));
            int byoyomiTimeSeconds = atoi(strtok(NULL, " "));
            int byoyomiPeriods = atoi(strtok(NULL, "\n"));
            bool ranked = atoi(strtok(NULL, "\n"));
            char *reply = strtok(NULL, "\n");

            ClientInfo *opponentInfo = findClientByUsername(opponent);

            pthread_mutex_lock(&ctxMutex);
            if (find(ctx[sock].challengers.begin(), ctx[sock].challengers.end(), opponentInfo->socket) ==
                ctx[sock].challengers.end()) {
                pthread_mutex_unlock(&ctxMutex);
                sendMessage(sock, "INVCCL", "");
                continue;
            }

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%s\n%s\n", ctx[sock].selfInfo->account->username.c_str(), reply);
            pthread_mutex_unlock(&ctxMutex);
            sendMessage(opponentInfo->socket, "INVRES", buff);

            if (strcmp(reply, "ACCEPT") == 0) {
                setupGame(sock, opponentInfo->socket,
                          new GoGame(boardSize, komi, timeSystem,
                                     mainTimeSeconds, byoyomiTimeSeconds,
                                     byoyomiPeriods, ranked));
            } else {
                pthread_mutex_lock(&ctxMutex);
                ctx[sock].opponentInfo = NULL;
                auto it = find(ctx[sock].challengers.begin(), ctx[sock].challengers.end(), opponentInfo->socket);
                if (it != ctx[sock].challengers.end())
                    ctx[sock].challengers.erase(it);
                pthread_mutex_unlock(&ctxMutex);
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

                    sendMessage(sock, "MOVE", buff);
                    pthread_mutex_lock(&ctxMutex);
                    if (ctx[sock].opponentInfo != NULL) {
                        usleep(10000);
                        sendMessage(ctx[sock].opponentInfo->socket, "MOVE", buff);
                    } else {
                        sendComputerMove(ctx[sock].selfInfo, game, 3 - color);
                    }
                    pthread_mutex_unlock(&ctxMutex);
                } else {
                    sendMessage(sock, "MOVERR", "");
                }
            } else {
                if (game->pass(color) == 2) {
                    getGameResult(buff, game);
                    sendMessage(sock, "RESULT", buff);
                    pthread_mutex_lock(&ctxMutex);
                    if (ctx[sock].opponentInfo != NULL) {
                        sendMessage(ctx[sock].opponentInfo->socket, "RESULT", buff);
                        updatePlayerRankings(game, ctx[sock].selfInfo->account, ctx[sock].opponentInfo->account);
                    } else {
                        updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                    }
                    pthread_mutex_unlock(&ctxMutex);
                    saveGame(game);
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
                            sendMessage(sock, "MOVE", buff);

                            getGameResult(buff, game);
                            sendMessage(sock, "RESULT", buff);
                            updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                            saveGame(game);
                            continue;
                        }
                        sendComputerMove(ctx[sock].selfInfo, game, 3 - color);
                        continue;
                    }
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\n%s\n", color, coords);
                    sendMessage(ctx[sock].opponentInfo->socket, "MOVE", buff);
                }
            }
            continue;
        }

        // Byo-yomi time control
        if (strcmp(messageType, "BYOYOM") == 0) {
            GoGame *game = ctx[sock].game;

            int color = atoi(strtok(payload, "\n"));
            char timingType = strtok(NULL, "\n")[0];
            int timeLeft = atoi(strtok(NULL, "\n"));

            if (timingType == 'M') {
                game->setMainTimeSecondsLeft(color, timeLeft);
                if (timeLeft > 0) {
                    int nextColor = 3 - color;
                    memset(buff, 0, BUFF_SIZE);
                    if (game->getMainTimeSecondsLeft(nextColor) > 0) {
                        sprintf(buff, "%d\nM\n%d\n", nextColor, game->getMainTimeSecondsLeft(nextColor));
                    } else {
                        sprintf(buff, "%d\nB\n%d\n", nextColor, game->getByoyomiTimeSeconds());
                    }
                    sendMessage(sock, "BYOYOM", buff);
                    if (ctx[sock].opponentInfo != NULL) {
                        sendMessage(ctx[sock].opponentInfo->socket, "BYOYOM", buff);
                    }
                } else {
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\nB\n%d\n", color, game->getByoyomiTimeSeconds());
                    sendMessage(sock, "BYOYOM", buff);
                    if (ctx[sock].opponentInfo != NULL) {
                        sendMessage(ctx[sock].opponentInfo->socket, "BYOYOM", buff);
                    }
                }
            } else {
                if (timeLeft > 0) {
                    int nextColor = 3 - color;
                    memset(buff, 0, BUFF_SIZE);
                    if (game->getMainTimeSecondsLeft(nextColor) > 0) {
                        sprintf(buff, "%d\nM\n%d\n", nextColor, game->getMainTimeSecondsLeft(nextColor));
                    } else {
                        sprintf(buff, "%d\nB\n%d\n", nextColor, game->getByoyomiTimeSeconds());
                    }
                    sendMessage(sock, "BYOYOM", buff);
                    if (ctx[sock].opponentInfo != NULL) {
                        sendMessage(ctx[sock].opponentInfo->socket, "BYOYOM", buff);
                    }
                } else {
                    if (game->getByoyomiPeriodsLeft(color) > 1) {
                        game->setByoyomiPeriodsLeft(color, game->getByoyomiPeriodsLeft(color) - 1);
                        memset(buff, 0, BUFF_SIZE);
                        sprintf(buff, "%d\nB\n%d\n", color, game->getByoyomiTimeSeconds());
                        sendMessage(sock, "BYOYOM", buff);
                        if (ctx[sock].opponentInfo != NULL) {
                            sendMessage(ctx[sock].opponentInfo->socket, "BYOYOM", buff);
                        }
                    } else {
                        memset(buff, 0, BUFF_SIZE);
                        sprintf(buff, "%d\nTIMEOUT\n", color);
                        sendMessage(sock, "INTRPT", buff);
                        if (ctx[sock].opponentInfo != NULL) {
                            sendMessage(ctx[sock].opponentInfo->socket, "INTRPT", buff);
                        }

                        game->timeout(color);

                        memset(buff, 0, BUFF_SIZE);
                        sprintf(buff, "%.1f %.1f\n%s\n%s\n", game->getBlackScore(), game->getWhiteScore(),
                                game->getBlackTerritory().c_str(), game->getWhiteTerritory().c_str());

                        sendMessage(sock, "RESULT", buff);

                        pthread_mutex_lock(&ctxMutex);
                        if (ctx[sock].opponentInfo != NULL) {
                            sendMessage(ctx[sock].opponentInfo->socket, "RESULT", buff);
                            updatePlayerRankings(game, ctx[sock].selfInfo->account,
                                                 ctx[sock].opponentInfo->account);
                        } else {
                            updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                        }
                        pthread_mutex_unlock(&ctxMutex);
                        saveGame(game);
                    }
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
                    sendMessage(ctx[sock].opponentInfo->socket, "INTRPT", buff);
                }

                game->resign(color);

                memset(buff, 0, BUFF_SIZE);
                sprintf(buff, "%.1f %.1f\n%s\n%s\n", game->getBlackScore(), game->getWhiteScore(),
                        game->getBlackTerritory().c_str(), game->getWhiteTerritory().c_str());

                usleep(10000);
                sendMessage(sock, "RESULT", buff);

                pthread_mutex_lock(&ctxMutex);
                if (ctx[sock].opponentInfo != NULL) {
                    sendMessage(ctx[sock].opponentInfo->socket, "RESULT", buff);
                    updatePlayerRankings(game, ctx[sock].selfInfo->account, ctx[sock].opponentInfo->account);
                } else {
                    updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                }
                pthread_mutex_unlock(&ctxMutex);
                saveGame(game);
                continue;
            }

            if (strcmp(interruptType, "DRAW") == 0) {
                pthread_mutex_lock(&ctxMutex);
                if (ctx[sock].opponentInfo == NULL) {
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\n%s\n%s\n", color, interruptType, "ACCEPT");
                    sendMessage(sock, "INTRES", buff);

                    game->acceptDraw(3 - color);

                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%.1f %.1f\n%s\n%s\n", game->getBlackScore(), game->getWhiteScore(),
                            game->getBlackTerritory().c_str(), game->getWhiteTerritory().c_str());

                    usleep(10000);
                    sendMessage(sock, "RESULT", buff);
                    updatePlayerRankings(game, ctx[sock].selfInfo->account, cpu);
                    saveGame(game);
                } else {
                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%d\n%s\n", color, interruptType);
                    sendMessage(ctx[sock].opponentInfo->socket, "INTRPT", buff);
                }
                pthread_mutex_unlock(&ctxMutex);
                continue;
            }

            if (strcmp(interruptType, "RESTART") == 0) {
                pthread_mutex_lock(&ctxMutex);
                if (ctx[sock].opponentInfo == NULL) {
                    game->reset();
                    setupGame(sock, -1, game);
                } else {
                    int oppsock = ctx[sock].opponentInfo->socket;
                    if (ctx[oppsock].game != game) {
                        memset(buff, 0, BUFF_SIZE);
                        sprintf(buff, "%d\n%s\n%s\n", color, interruptType, "LEFT");
                        sendMessage(sock, "INTRES", buff);
                    } else {
                        memset(buff, 0, BUFF_SIZE);
                        sprintf(buff, "%d\n%s\n", color, interruptType);
                        sendMessage(oppsock, "INTRPT", buff);
                    }
                }
                pthread_mutex_unlock(&ctxMutex);
            }

            continue;
        }

        // Respond to interrupt
        if (strcmp(messageType, "INTRES") == 0) {
            int color = atoi(strtok(payload, "\n"));
            char *interruptType = strtok(NULL, "\n");
            char *reply = strtok(NULL, "\n");

            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%d\n%s\n%s\n", color, interruptType, reply);
            sendMessage(ctx[sock].opponentInfo->socket, "INTRES", buff);

            GoGame *game = ctx[sock].game;

            if (strcmp(interruptType, "DRAW") == 0) {
                if (strcmp(reply, "ACCEPT") == 0) {
                    game->acceptDraw(3 - color);

                    memset(buff, 0, BUFF_SIZE);
                    sprintf(buff, "%.1f %.1f\n%s\n%s\n", game->getBlackScore(), game->getWhiteScore(),
                            game->getBlackTerritory().c_str(), game->getWhiteTerritory().c_str());

                    usleep(10000);
                    sendMessage(sock, "RESULT", buff);

                    sendMessage(ctx[sock].opponentInfo->socket, "RESULT", buff);
                    updatePlayerRankings(game, ctx[sock].selfInfo->account, ctx[sock].opponentInfo->account);
                    saveGame(game);
                }
            } else if (strcmp(interruptType, "RESTART") == 0) {
                if (strcmp(reply, "ACCEPT") == 0) {
                    game->reset();
                    setupGame(sock, ctx[sock].opponentInfo->socket, game);
                }
            }
            continue;
        }

        // Confirm game result
        if (strcmp(messageType, "RESACK") == 0) {
            pthread_mutex_lock(&ctxMutex);
            ctx[sock].selfInfo->status = 0;
            ctx[sock].opponentInfo = NULL;
            ctx[sock].game = NULL;
            pthread_mutex_unlock(&ctxMutex);
            notifyOnlineStatusChange();
            continue;
        }

        // Get history of played games
        if (strcmp(messageType, "HISTRY") == 0) {
            vector < GameRecord * > games = findGamesByPlayer(ctx[sock].selfInfo->account->id);
            memset(payload, 0, 16 * BUFF_SIZE);
            memset(buff, 0, BUFF_SIZE);
            for (const GameRecord *game: games) {
                sprintf(buff, "%s %d %d %s %.2f %.2f %d %ld\n", game->id.c_str(), game->boardSize,
                        game->color, game->opponent.c_str(),
                        game->blackScore, game->whiteScore,
                        game->eloChange, game->time);
                strcat(payload, buff);
            }
            strcat(payload, "\0");
            sendMessage(sock, "HISTRY", payload);
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
            sendMessage(sock, "REPLAY", data);
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
            sendMessage(sock, "RANKIN", data.c_str());
            continue;
        }

        // Get stats
        if (strcmp(messageType, "STATS") == 0) {
            int playerId = ctx[sock].selfInfo->account->id;
            Stats *stats = getStatsOfPlayer(playerId);
            memset(buff, 0, BUFF_SIZE);
            sprintf(buff, "%d %d %d %d %.2f%% %d %s %d\n",
                    stats->totalMatches, stats->wins, stats->losses, stats->draws, stats->winningRate * 100,
                    stats->elo, stats->rankType.c_str(), stats->ranking);
            sendMessage(sock, "STATS", buff);
            continue;
        }

        // Log out
        if (strcmp(messageType, "LOGOUT") == 0) {
            handleClientDisconnect(sock);
            continue;
        }
    }

    close(sock);
    return NULL;
}

#endif //GAME_OF_GO_SERVER_CONTROLLER_HPP
