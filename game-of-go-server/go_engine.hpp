//
// Created by Admin on 11/21/2023.
//

#ifndef GAME_OF_GO_SERVER_GO_ENGINE_HPP
#define GAME_OF_GO_SERVER_GO_ENGINE_HPP

#include <stdio.h>
#include <vector>
#include <queue>
#include <set>
#include <string>
#include <algorithm>

using namespace std;

#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define MARKER 4
#define OFFBOARD 7
#define LIBERTY 8

string symbol = ".#o .bw +";

class GoGame {
private:
    int boardSize;
    int boardRange;
    vector<int> board;
    vector<int> liberties;
    vector<int> block;
    vector<int> captured[3];
    float score[3];
    int consecutivePass;
    int lastMove = -1;
    vector<int> lastCaptured;

    void count(int pos, int color) {
        int piece = board[pos];

        if (piece == OFFBOARD)
            return;

        if (piece && (piece & color) && !(piece & MARKER)) {
            block.push_back(pos);
            board[pos] |= MARKER;

            count(pos - boardRange, color);
            count(pos - 1, color);
            count(pos + boardRange, color);
            count(pos + 1, color);
        } else if (piece == EMPTY) {
            board[pos] |= LIBERTY;
            liberties.push_back(pos);
        }
    }

    void clearBlock() {
        for (int captured: block) {
            board[captured] = EMPTY;
        }
    }

    void clearGroups() {
        block.clear();
        liberties.clear();
    }

    void restoreBoard() {
        clearGroups();
        for (int pos = 0; pos < boardRange * boardRange; pos++) {
            if (board[pos] != OFFBOARD)
                board[pos] &= 3;
        }
    }

    void clearBoard() {
        clearGroups();
        for (int pos = 0; pos < board.size(); pos++) {
            if (board[pos] != OFFBOARD)
                board[pos] = EMPTY;
        }
    }

    void capture(int color) {
        captured[0].clear();

        for (int pos = 0; pos < board.size(); pos++) {
            int piece = board[pos];

            if (piece == OFFBOARD) continue;

            if (piece & color) {
                count(pos, color);
                if (liberties.size() == 0) {
                    copy(block.begin(), block.end(), back_inserter(captured[0]));
                    copy(block.begin(), block.end(), back_inserter(captured[color]));
                    clearBlock();
                }
                restoreBoard();
            }
        }
    }

    int canMove(int pos, int color) {
        // check if the move can capture any opponent stones
        board[pos] = color;
        int directions[] = {-1, 1, -boardRange, boardRange};
        int capturable = 0;

        for (int dir: directions) {
            count(pos + dir, 3 - color);
            if (liberties.size() == 0) {
                restoreBoard();
                capturable = 1;
                break;
            }
            restoreBoard();
        }

        // check if the position has any liberties
        int hasLiberties = 1;
        count(pos, color);
        if (liberties.size() == 0) {
            hasLiberties = 0;
        }
        restoreBoard();

        if (!capturable && !hasLiberties) {
            board[pos] = EMPTY;
            return 0;
        }

        // check if the move breaks Ko rule
        board[pos] = color;
        capture(3 - color);
        if (captured[0].size() == 1 && lastCaptured.size() == 1
            && captured[0][0] == lastMove && pos == lastCaptured[0]) {
            board[captured[0][0]] = 3 - color;
            board[pos] = EMPTY;
            return 0;
        }
        for (int cap: captured[0]) {
            board[cap] = 3 - color;
        }
        board[pos] = EMPTY;
        return 1;
    }

    void determineTerritory(int pos) {
        set<int> black;
        set<int> white;
        set<int> side;
        set<int> visited;
        queue<int> q;

        q.push(pos);

        while (!q.empty()) {
            int p = q.front();
            q.pop();

            if (board[p] == OFFBOARD) {
                if (p / boardRange == 0) side.insert(1);
                if (p / boardRange == boardSize + 1) side.insert(2);
                if (p % boardRange == 0) side.insert(3);
                if (p % boardRange == boardSize + 1) side.insert(4);
            } else if (board[p] == BLACK)
                black.insert(p);
            else if (board[p] == WHITE)
                white.insert(p);
            else {
                if (visited.find(p) != visited.end()) continue;
                visited.insert(p);

                q.push(p + 1);
                q.push(p - 1);
                q.push(p + boardRange);
                q.push(p - boardRange);
            }
        }

        int marker;
        if (side.size() == 4 || black.size() == white.size()) {
            marker = MARKER;
        } else if (black.size() > white.size()) {
            marker = BLACK | MARKER;
        } else if (black.size() < white.size()) {
            marker = WHITE | MARKER;
        }

        for (int p: visited) {
            board[p] = marker;
        }
    }

public:
    GoGame(int boardSize) {
        this->boardSize = boardSize;
        int boardRange = boardSize + 2;
        this->boardRange = boardRange;
        this->board.resize(boardRange * boardRange);

        for (int i = 0; i < boardRange * boardRange; i++) {
            int row = i / boardRange;
            int col = i % boardRange;
            if (row < 1 || row > boardSize || col < 1 || col > boardSize)
                board[i] = 7;
            else
                board[i] = 0;
        }
    }

    void printBoard() {
        for (int row = 0; row < boardRange; row++) {
            for (int col = 0; col < boardRange; col++) {
                int pos = row * boardRange + col;
                printf("%c ", symbol[board[pos]]);
            }
            printf("\n");
        }
    }

    int play(string coords, int color) {
        consecutivePass = 0;

        char colSymbol = coords[0];
        int col = colSymbol - 'A' + 1;
        if (colSymbol >= 'J') col--;
        int row = stoi(coords.substr(1));

        int pos = row * boardRange + col;
        if (board[pos] != EMPTY)
            return 0;

        if (canMove(pos, color)) {
            board[pos] = color;
            capture(3 - color);
            printBoard();
            lastMove = pos;
            lastCaptured = captured[0];
            return 1;
        } else {
            return 0;
        }
    }

    int pass() {
        consecutivePass++;
        return consecutivePass;
    }

    vector <string> getCaptured() {
        vector <string> captured;
        for (int pos: this->captured[0]) {
            int row = pos / boardRange;
            int col = pos % boardRange;

            char colSymbol = col - 1 + 'A';
            if (colSymbol >= 'I') colSymbol++;
            captured.push_back(string(1, colSymbol) + to_string(row));
        }
        return captured;
    }

    pair<float, float> calculateScore() {
        for (int pos = 0; pos < boardRange * boardRange; pos++) {
            if (board[pos] == EMPTY) {
                determineTerritory(pos);
            }
        }

        float blackScore = 0.0;
        float whiteScore = 6.5;

        for (int pos = 0; pos < boardRange * boardRange; pos++) {
            if (board[pos] == (MARKER | BLACK)) blackScore = blackScore + 1;
            else if (board[pos] == (MARKER | WHITE)) whiteScore = whiteScore + 1;
        }

        blackScore = blackScore + captured[WHITE].size();
        whiteScore = whiteScore + captured[BLACK].size();

        printBoard();

        return make_pair(blackScore, whiteScore);
    }
};

#endif //GAME_OF_GO_SERVER_GO_ENGINE_HPP
