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

#endif