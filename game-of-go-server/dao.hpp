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
            "INSERT INTO account (username, password, points) VALUES (?, ?, 0)");
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
                                       rs->getInt("points"));
        return account;
    } else {
        return NULL;
    }
}

void saveGame(GoGame *game) {
    auto pstmt = con->prepareStatement(
            "INSERT INTO game (id, time, board_size, black_player, white_player, log, black_score, white_score, black_territory, white_territory) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
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
    pstmt->executeUpdate();
}

vector<GameRecord *> findGamesByPlayer(int playerId) {
    auto pstmt = con->prepareStatement(
            "SELECT g.id, g.board_size, p1.username AS black_player, p2.username AS white_player, black_score, white_score, time "
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
        GameRecord *game = new GameRecord(
                rs->getString("id"),
                rs->getInt("board_size"),
                rs->getString("black_player"),
                rs->getString("white_player"),
                rs->getDouble("black_score"),
                rs->getDouble("white_score"),
                rs->getInt64("time")
        );
        games.push_back(game);
    }

    return games;
}

GameReplay *getGameReplayInfo(string gameId) {
    auto pstmt = con->prepareStatement(
            "SELECT log, black_territory, white_territory FROM game WHERE id = ?");
    pstmt->setString(1, gameId);
    auto rs = pstmt->executeQuery();

    if (rs->next()) {
        return new GameReplay(
                rs->getString("log"),
                rs->getString("black_territory"),
                rs->getString("white_territory")
        );
    }
    return NULL;
}

#endif