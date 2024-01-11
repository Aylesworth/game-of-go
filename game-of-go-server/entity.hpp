#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <string>

#define MAX 255
using namespace std;

struct Account {
    int id;
    string username;
    string password;
    int elo;
    string rankType;

    Account() {}

    Account(int id, string username, string password, int elo, string rankType) :
            id(id), username(username), password(password), elo(elo), rankType(rankType) {}
};

struct GameRecord {
    string id;
    int boardSize;
    int color;
    string opponent;
    double blackScore;
    double whiteScore;
    int eloChange;
    int64_t time;

    GameRecord() {}

    GameRecord(string id, int boardSize, int color, string opponent,
               double blackScore, double whiteScore, int eloChange, int64_t time) :
            id(id), boardSize(boardSize), color(color), opponent(opponent),
            blackScore(blackScore), whiteScore(whiteScore),
            eloChange(eloChange), time(time) {}
};

struct GameReplay {
    string id;
    string log;
    string blackTerritory;
    string whiteTerritory;

    GameReplay() {}

    GameReplay(string id, string log, string blackTerritory, string whiteTerritory) :
            id(id), log(log), blackTerritory(blackTerritory), whiteTerritory(whiteTerritory) {}
};

struct Stats {
    int totalMatches;
    int wins;
    int losses;
    int draws;
    double winningRate;
    int elo;
    string rankType;
    int ranking;

    Stats() {}

    Stats(int totalMatches, int wins, int losses, int draws, double winningRate, int elo, string rankType, int ranking) :
            totalMatches(totalMatches), wins(wins), losses(losses), draws(draws), winningRate(winningRate),
            elo(elo), rankType(rankType), ranking(ranking) {}
};

#endif