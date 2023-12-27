//
// Created by Admin on 11/16/2023.
//

#ifndef GAME_OF_GO_SERVER_SERVICE_HPP
#define GAME_OF_GO_SERVER_SERVICE_HPP

#include <string>
#include <string.h>
#include <openssl/md5.h>
#include <time.h>
#include "entity.hpp"
#include "dao.hpp"

using namespace std;

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

int handleCreateAccount(char *payload) {
    char *username = strtok(payload, "\n");
    char *password = strtok(NULL, "\n");

    if (findAccount(username) != NULL)
        return 1;

    Account account;
    account.username = username;
    account.password = encodeMD5(password);

    createAccount(account);
    return 0;
}

Account *handleSignIn(char *payload) {
    char *username = strtok(payload, "\n");
    char *password = strtok(NULL, "\n");

    Account *account = findAccount(username);
    if (account == NULL)
        return NULL;

    string passwordHash = encodeMD5(password);
    if (passwordHash != account->password)
        return NULL;

    account->password = "";
    return account;
}

string generateGameId() {
    string id;
    vector<char> pool;
    for (char c = 'A'; c <= 'Z'; c++) pool.push_back(c);
    for (char c = 'a'; c <= 'z'; c++) pool.push_back(c);
    for (char c = '0'; c <= '9'; c++) pool.push_back(c);

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

void handleSaveGame(GoGame *game) {
    printf("Saving game %s\n", game->getId().c_str());
    saveGame(game);
}

void handleUpdateRanking(Account account) {
    updateRanking(account);
}

vector<GameRecord *> handleGetHistory(int playerId) {
    return findGamesByPlayer(playerId);
}

GameReplay *handleGetReplay(string gameId) {
    return getGameReplayInfo(gameId);
}

vector<Account *> getRandomPlayers(int quantity) {
    return randomPlayers(quantity);
}

#endif //GAME_OF_GO_SERVER_SERVICE_HPP
