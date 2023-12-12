#ifndef ENTITY_HPP
#define ENTITY_HPP

#include<string>

#define MAX 255
using namespace std;

struct Account {
    int id;
    string username;
    string password;
    int points;

    Account() {}

    Account(int id, string username, string password, int points) :
            id(id), username(username), password(password), points(points) {}
};

struct GameRecord {
    string id;
    int boardSize;
    string blackPlayer;
    string whitePlayer;
    double blackScore;
    double whiteScore;
    int64_t time;

    GameRecord() {}

    GameRecord(string id, int boardSize, string blackPlayer, string whitePlayer,
               double blackScore, double whiteScore, int64_t time) :
            id(id), boardSize(boardSize), blackPlayer(blackPlayer), whitePlayer(whitePlayer),
            blackScore(blackScore), whiteScore(whiteScore), time(time) {}
};

#endif