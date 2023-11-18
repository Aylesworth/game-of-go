#ifndef DAO_HPP
#define DAO_HPP

#include "entity.hpp"
#include <string>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>
#include <stdlib.h>

sql::Connection *getConnection() {
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *con;

    driver = sql::mysql::get_mysql_driver_instance();
    con = driver->connect("tcp://localhost:3306", "nda2105", "Ducanh2003@");
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
        Account *account = new Account(rs->getInt("id"), rs->getString("username"), rs->getString("password"), rs->getInt("points"));
        return account;
    } else {
        return NULL;
    }
}

#endif