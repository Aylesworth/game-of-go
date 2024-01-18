#ifndef DAO_HPP
#define DAO_HPP

#include "entity.hpp"
#include "go_engine.hpp"
#include <string>
#include <vector>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <stdlib.h>

using namespace std;

sql::Connection *getConnection() {
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;

    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://localhost:3306", "nda2105", "ducanh2003");
    con->setSchema("game_of_go");

    return con;
}

sql::Connection *con = getConnection();

void createAccount(Account account) {
    auto pstmt = con->prepareStatement(
            "INSERT INTO account (username, password, elo, rank_type) VALUES (?, ?, 1000, '11K')");
    pstmt->setString(1, account.username);
    pstmt->setString(2, account.password);
    pstmt->executeUpdate();
}

Account *findAccount(string username) {
    auto pstmt = con->prepareStatement("SELECT * FROM account WHERE username = ?");
    pstmt->setString(1, username);

    auto rs = pstmt->executeQuery();

    if (rs->next()) {
        Account *account = new Account(rs->getInt("id"), rs->getString("username"), rs->getString("password"),
                                       rs->getInt("elo"), rs->getString("rank_type"));
        return account;
    } else {
        return NULL;
    }
}

vector<Account *> getRandomPlayers(int quantity) {
    auto pstmt = con->prepareStatement("SELECT * FROM account ORDER BY RAND() LIMIT ?");
    pstmt->setInt(1, quantity);
    auto rs = pstmt->executeQuery();

    vector < Account * > players;
    while (rs->next()) {
        players.push_back(new Account(rs->getInt("id"), rs->getString("username"), rs->getString("password"),
                                      rs->getInt("elo"), rs->getString("rank_type")));
    }

    return players;
}

void updateRanking(Account account) {
    auto pstmt = con->prepareStatement(
            "UPDATE account SET elo = ?, rank_type = ? WHERE id = ?");
    pstmt->setInt(1, account.elo);
    pstmt->setString(2, account.rankType);
    pstmt->setInt(3, account.id);
    pstmt->executeUpdate();
}

vector <pair<int, Account *>> getRankings() {
    auto pstmt = con->prepareStatement(
            "SELECT DENSE_RANK() OVER (ORDER BY elo DESC) AS ranking, username, elo, rank_type "
            "FROM account ORDER BY elo DESC");
    auto rs = pstmt->executeQuery();

    vector <pair<int, Account *>> rankings;
    while (rs->next()) {
        rankings.push_back(make_pair(
                rs->getInt("ranking"),
                new Account(
                        0,
                        rs->getString("username"),
                        "",
                        rs->getInt("elo"),
                        rs->getString("rank_type")
                )
        ));
    }

    return rankings;
}

bool doesGameIdExist(string id) {
    auto pstmt = con->prepareStatement("SELECT * FROM game WHERE id = ?");
    pstmt->setString(1, id);
    auto rs = pstmt->executeQuery();
    return rs->next();
}

void saveGame(GoGame *game) {
    auto pstmt = con->prepareStatement(
            "INSERT INTO game (id, time, board_size, black_player, white_player, log, black_score, white_score, black_territory, white_territory, black_elo_change, white_elo_change) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    pstmt->setString(1, game->getId());
    pstmt->setInt64(2, game->getTimestamp());
    pstmt->setInt(3, game->getBoardSize());
    pstmt->setInt(4, game->getBlackPlayerId());
    pstmt->setInt(5, game->getWhitePlayerId());
    pstmt->setString(6, game->getLog());
    pstmt->setDouble(7, game->getBlackScore());
    pstmt->setDouble(8, game->getWhiteScore());
    pstmt->setString(9, game->getBlackTerritory());
    pstmt->setString(10, game->getWhiteTerritory());
    pstmt->setInt(11, game->getBlackEloChange());
    pstmt->setInt(12, game->getWhiteEloChange());
    pstmt->executeUpdate();
}

vector<GameRecord *> findGamesByPlayer(int playerId) {
    auto pstmt = con->prepareStatement(
            "SELECT g.id, g.board_size, p1.id AS black_id, p1.username AS black_name, p2.id AS white_id, p2.username AS white_name, "
            "g.black_score, g.white_score, g.black_elo_change, g.white_elo_change, g.time "
            "FROM game g "
            "JOIN account p1 ON p1.id = g.black_player "
            "JOIN account p2 ON p2.id = g.white_player "
            "WHERE g.black_player = ? OR g.white_player = ? "
            "ORDER BY time DESC");
    pstmt->setInt(1, playerId);
    pstmt->setInt(2, playerId);
    auto rs = pstmt->executeQuery();

    vector < GameRecord * > games;
    while (rs->next()) {
        int color;
        string opponent;
        int eloChange;
        if (rs->getInt("black_id") == playerId) {
            color = 1;
            opponent = rs->getString("white_name");
            eloChange = rs->getInt("black_elo_change");
        } else {
            color = 2;
            opponent = rs->getString("black_name");
            eloChange = rs->getInt("white_elo_change");
        }
        GameRecord *game = new GameRecord(
                rs->getString("id"),
                rs->getInt("board_size"),
                color,
                opponent,
                rs->getDouble("black_score"),
                rs->getDouble("white_score"),
                eloChange,
                rs->getInt64("time")
        );
        games.push_back(game);
    }

    return games;
}

GameReplay *getGameReplayInfo(string gameId) {
    auto pstmt = con->prepareStatement(
            "SELECT g.id, g.log, g.black_territory, g.white_territory, p1.username AS black_name, p2.username AS white_name "
            "FROM game g "
            "JOIN account p1 ON p1.id = g.black_player "
            "JOIN account p2 ON p2.id = g.white_player "
            "WHERE g.id = ?");
    pstmt->setString(1, gameId);
    auto rs = pstmt->executeQuery();

    if (rs->next()) {
        return new GameReplay(
                rs->getString("id"),
                rs->getString("black_name"),
                rs->getString("white_name"),
                rs->getString("log"),
                rs->getString("black_territory"),
                rs->getString("white_territory")
        );
    }
    return NULL;
}

Stats *getStatsOfPlayer(int playerId) {
    int totalMatches, wins, losses, draws;
    double winningRate;
    int elo;
    string rankType;
    int ranking;

    // Get total matches
    auto pstmt = con->prepareStatement(
            "SELECT COUNT(id) AS total_matches FROM game "
            "WHERE black_player = ? OR white_player = ?"
    );
    pstmt->setInt(1, playerId);
    pstmt->setInt(2, playerId);
    auto rs = pstmt->executeQuery();
    if (rs->next()) {
        totalMatches = rs->getInt("total_matches");
    } else return NULL;

    // Get wins
    pstmt = con->prepareStatement(
            "SELECT COUNT(id) AS wins FROM game "
            "WHERE (black_player = ? AND black_score > white_score) "
            "OR (white_player = ? AND white_score > black_score)"
    );
    pstmt->setInt(1, playerId);
    pstmt->setInt(2, playerId);
    rs = pstmt->executeQuery();
    if (rs->next()) {
        wins = rs->getInt("wins");
    } else return NULL;

    // Get losses
    pstmt = con->prepareStatement(
            "SELECT COUNT(id) AS losses FROM game "
            "WHERE (black_player = ? AND black_score < white_score) "
            "OR (white_player = ? AND white_score < black_score)"
    );
    pstmt->setInt(1, playerId);
    pstmt->setInt(2, playerId);
    rs = pstmt->executeQuery();
    if (rs->next()) {
        losses = rs->getInt("losses");
    } else return NULL;

    // Get draws
    draws = totalMatches - wins - losses;

    // Calculate winning rate
    winningRate = 1.0 * wins / totalMatches;

    // Get ELO rating and rank type
    pstmt = con->prepareStatement(
            "SELECT elo, rank_type FROM account "
            "WHERE id = ?"
    );
    pstmt->setInt(1, playerId);
    rs = pstmt->executeQuery();
    if (rs->next()) {
        elo = rs->getInt("elo");
        rankType = rs->getString("rank_type");
    } else return NULL;

    // Get ranking
    pstmt = con->prepareStatement(
            "SELECT ranking FROM "
            "(SELECT DENSE_RANK() OVER (ORDER BY elo DESC) AS ranking, id FROM account) AS rankings "
            "WHERE id = ?"
    );
    pstmt->setInt(1, playerId);
    rs = pstmt->executeQuery();
    if (rs->next()) {
        ranking = rs->getInt("ranking");
    } else return NULL;

    return new Stats(totalMatches, wins, losses, draws, winningRate, elo, rankType, ranking);
}

#endif