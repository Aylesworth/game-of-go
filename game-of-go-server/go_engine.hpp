//
// Created by Admin on 11/21/2023.
//

#ifndef GAME_OF_GO_SERVER_GO_ENGINE_HPP
#define GAME_OF_GO_SERVER_GO_ENGINE_HPP

#include <stdio.h>
#include <time.h>
#include <vector>
#include <queue>
#include <set>
#include <string>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

#define EMPTY 0
#define BLACK 1
#define WHITE 2
#define MARKER 4
#define LIBERTY 8
#define OFFBOARD 12

#define NONE 0
#define BYOYOMI 1

class GoGame {
private:
    string id;
    int64_t timestamp;
    int blackPlayerId;
    int whitePlayerId;
    int boardSize;
    float komi;
    int timeSystem;
    int mainTimeSeconds[3];
    int byoyomiPeriods[3];
    int byoyomiTimeSeconds;
    int ranked;
    int boardRange;
    vector<int> board;
    vector<int> liberties;
    vector<int> block;
    vector<int> captured[3];
    int consecutivePass;
    int lastMove = -1;
    vector<int> lastCaptured;
    string log;
    float blackScore;
    float whiteScore;
    string blackTerritory;
    string whiteTerritory;

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
                    clearBlock();
                }
                restoreBoard();
            }
        }
    }

    int canMove(int pos, int color) {
        // check if the move can capture any opponent stones
        if (board[pos] != EMPTY) return 0;

        board[pos] = color;
        int directions[] = {-1, 1, -boardRange, boardRange};
        int capturable = 0;

        for (int dir: directions) {
            if (board[pos + dir] != 3 - color) continue;
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

    int detectsEdge(int pos) {
        int directions[] = {boardRange, 1, -boardRange, -1};
        for (int dir: directions) {
            if (board[pos + dir] == OFFBOARD) return 1;
        }
        return 0;
    }

    int evaluate(int color) {
        int bestCount = 0;
        int bestLiberty = liberties[0];

        vector<int> libertiesCopy = liberties;
        for (int liberty: libertiesCopy) {
            board[liberty] = color;
            count(liberty, color);
            if (liberties.size() > bestCount) {
                bestLiberty = liberty;
                bestCount = liberties.size();
            }
            restoreBoard();
            board[liberty] = EMPTY;
        }

        return bestLiberty;
    }

    int libertiesIfPut(int pos, int color) {
        board[pos] = color;
        count(pos, color);
        int n = liberties.size();
        restoreBoard();
        board[pos] = EMPTY;
        return n;
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
            marker = MARKER | BLACK;
        } else if (black.size() < white.size()) {
            marker = MARKER | WHITE;
        }

        for (int p: visited) {
            board[p] = marker;
        }
    }

public:
    GoGame(int boardSize,
           float komi,
           int timeSystem,
           int mainTimeSeconds,
           int byoyomiTimeSeconds,
           int byoyomiPeriods,
           int ranked
    ) {
        this->timestamp = (int64_t) time(NULL);
        this->boardSize = boardSize;
        this->komi = komi;
        this->timeSystem = timeSystem;
        this->mainTimeSeconds[BLACK] = mainTimeSeconds;
        this->mainTimeSeconds[WHITE] = mainTimeSeconds;
        this->byoyomiPeriods[BLACK] = byoyomiPeriods;
        this->byoyomiPeriods[WHITE] = byoyomiPeriods;
        this->byoyomiTimeSeconds = byoyomiTimeSeconds;
        this->ranked = ranked;

        int boardRange = boardSize + 2;
        this->boardRange = boardRange;
        this->board.resize(boardRange * boardRange);

        for (int i = 0; i < boardRange * boardRange; i++) {
            int row = i / boardRange;
            int col = i % boardRange;
            if (row < 1 || row > boardSize || col < 1 || col > boardSize)
                board[i] = OFFBOARD;
            else
                board[i] = EMPTY;
        }
    }

    string toCoords(int pos) {
        int row = pos / boardRange;
        int col = pos % boardRange;

        char colSymbol = col - 1 + 'A';
        if (colSymbol >= 'I') colSymbol++;

        return string(1, colSymbol) + to_string(row);
    }

    void printBoard() {
        string symbol = ".#o .bw +    ";

        printf("\n");
        for (int row = 0; row < boardRange; row++) {
            for (int col = 0; col < boardRange; col++) {
                int pos = row * boardRange + col;
                printf("%c ", symbol[board[pos]]);
            }
            printf("\n");
        }
        printf("\n");
    }

    int play(string coords, int color) {
        char colSymbol = coords[0];
        int col = colSymbol - 'A' + 1;
        if (colSymbol >= 'J') col--;
        int row = stoi(coords.substr(1));
        int pos = row * boardRange + col;

        if (canMove(pos, color)) {
            consecutivePass = 0;
            board[pos] = color;
            capture(3 - color);
            printBoard();
            lastMove = pos;
            lastCaptured = captured[0];
            copy(lastCaptured.begin(), lastCaptured.end(), back_inserter(captured[3 - color]));

            log += to_string(color) + "+" + toCoords(lastMove);
            if (lastCaptured.size() > 0) {
                log += "/" + to_string(3 - color) + "-";
                for (int i = 0; i < lastCaptured.size(); i++) {
                    if (i > 0) log += ",";
                    log += toCoords(lastCaptured[i]);
                }
            }
            log += " ";
            return 1;
        }
        return 0;
    }

    string randomMove(int color) {
        srand(time(NULL));
        int pos;
        string coords;
        int maxAttempts = 50;
        int count = 0;
        do {
            pos = rand() % (boardRange * boardRange);
            coords = toCoords(pos);
            count++;
        } while (count < maxAttempts && !play(coords, color));

        if (count >= maxAttempts) {
            return "PA";
        }

        return coords;
    }

    string generateMove(int color) {
        if (lastMove == -1) {
            return randomMove(color);
        }

        int bestMove = 0;
        int capture = 0;
        int save = 0;
        int defend = 0;
        int surround = 0;
        int pattern = 0;

        vector<int> candidates;
        srand(time(NULL));

        // capture opponent's group
        for (int pos = 0; pos < board.size(); pos++) {
            int piece = board[pos];
            if (piece & (3 - color)) {
                count(pos, 3 - color);
                if (liberties.size() == 1) {
                    int targetPos = liberties[0];
                    restoreBoard();
                    if (canMove(targetPos, color)) {
                        bestMove = targetPos;
                        candidates.push_back(targetPos);
                        break;
                    }
                }
                restoreBoard();
            }
        }
        if (candidates.size() > 0) {
            capture = candidates[rand() % candidates.size()];
            candidates.clear();
        }

        // save own group
        for (int pos = 0; pos < board.size(); pos++) {
            int piece = board[pos];
            if (piece & color) {
                count(pos, color);
                if (liberties.size() == 1) {
                    int targetPos = liberties[0];
                    restoreBoard();
                    if (canMove(targetPos, color) && libertiesIfPut(targetPos, color) > 1) {
                        bestMove = targetPos;
                        candidates.push_back(targetPos);
                        break;
                    }
                }
                restoreBoard();
            }
        }
        if (candidates.size() > 0) {
            save = candidates[rand() % candidates.size()];
            candidates.clear();
        }

        // defend own group
        for (int pos = 0; pos < board.size(); pos++) {
            int piece = board[pos];
            if (piece & color) {
                count(pos, color);
                if (liberties.size() == 2) {
                    int bestLiberty = evaluate(color);
                    restoreBoard();
                    if (canMove(bestLiberty, color) && libertiesIfPut(bestLiberty, color) > 1) {
                        bestMove = bestLiberty;
                        candidates.push_back(bestLiberty);
                        break;
                    }
                }
                restoreBoard();
            }
        }
        if (candidates.size() > 0) {
            defend = candidates[rand() % candidates.size()];
            candidates.clear();
        }

        // surround opponent's group
        for (int pos = 0; pos < board.size(); pos++) {
            int piece = board[pos];
            if (piece & (3 - color)) {
                count(pos, 3 - color);
                if (liberties.size() > 1) {
                    int bestLiberty = evaluate(3 - color);
                    restoreBoard();
                    if (canMove(bestLiberty, color) && libertiesIfPut(bestLiberty, color) > 1) {
                        bestMove = bestLiberty;
                        candidates.push_back(bestLiberty);
                        break;
                    }
                }
                restoreBoard();
            }
        }
        if (candidates.size() > 0) {
            surround = candidates[rand() % candidates.size()];
            candidates.clear();
        }

        // pattern matching
        int targetOne, targetTwo;
        for (int pos = 0; pos < board.size(); pos++) {
            int piece = board[pos];
            if (piece == OFFBOARD) continue;
            if (piece & (3 - color)) {
                targetOne = pos - boardRange + 1;
                targetTwo = pos - boardRange - 1;
                if ((board[targetOne] & color) && (board[targetTwo] & color) && canMove(pos - boardRange, color)) {
                    bestMove = pos - boardRange;
                    candidates.push_back(bestMove);
                }

                targetOne = pos + 1;
                targetTwo = pos - boardRange - 1;
                if ((board[targetOne] & color) && (board[targetTwo] & color) && canMove(pos - boardRange, color)) {
                    bestMove = pos - boardRange;
                    candidates.push_back(bestMove);
                }

                targetOne = pos + 1;
                targetTwo = pos - 1;
                if ((board[targetOne] & color) && (board[targetTwo] & color) && canMove(pos + boardRange, color)) {
                    bestMove = pos + boardRange;
                    candidates.push_back(bestMove);
                }

                targetOne = pos - boardRange + 2;
                targetTwo = pos - boardRange - 1;
                if ((board[targetOne] & color) && (board[targetTwo] & color) && canMove(pos - boardRange, color)) {
                    bestMove = pos - boardRange;
                    candidates.push_back(bestMove);
                }

                targetOne = pos - boardRange + 2;
                targetTwo = pos - boardRange - 2;
                if ((board[targetOne] & color) && (board[targetTwo] & color) && canMove(pos - boardRange, color)) {
                    bestMove = pos - boardRange;
                    candidates.push_back(bestMove);
                }

                targetOne = pos - 1;
                targetTwo = pos + boardRange - 2;
                if ((board[targetOne] & color) && (board[targetTwo] & color) && canMove(pos + boardRange, color)) {
                    bestMove = pos + boardRange;
                    candidates.push_back(bestMove);
                }

                targetOne = pos - boardRange;
                targetTwo = pos - boardRange - 2;
                if ((board[targetOne] & color) && (board[targetTwo] & color) && canMove(pos - 1, color)) {
                    bestMove = pos - 1;
                    candidates.push_back(bestMove);
                }

                if (candidates.size() > 0) {
                    pattern = candidates[rand() % candidates.size()];
                    candidates.clear();
                }
            }
        }

        if (bestMove) {
            printf("capture move: %s\n", toCoords(capture).c_str());
            printf("save move: %s\n", toCoords(save).c_str());
            printf("defend move: %s\n", toCoords(defend).c_str());
            printf("surround move: %s\n", toCoords(surround).c_str());
            printf("pattern move: %s\n", toCoords(pattern).c_str());

            if (save) bestMove = save;
            else if (capture) bestMove = capture;
            else {
                vector<int> possibleMoves;
                if (defend) possibleMoves.push_back(defend);
                if (surround) possibleMoves.push_back(surround);
                if (pattern) possibleMoves.push_back(pattern);

                srand(time(NULL));
                int random = rand() % possibleMoves.size();
                bestMove = possibleMoves[random];
            }

            string coords = toCoords(bestMove);
            printf("chosen move: %s\n", coords.c_str());
            play(coords, color);
            return coords;
        } else {
            srand(time(NULL));
            int random = rand() % 4;
            string move = !random ? "PA" : randomMove(color);
            printf("move: %s\n", move.c_str());
            return move;
        }
    }

    int pass(int color) {
        consecutivePass++;
        log += to_string(color) + "=PA ";
        return consecutivePass;
    }

    void resign(int color) {
        calculateScore();
        if (color == BLACK) blackScore = -1;
        else whiteScore = -1;
        log += to_string(color) + "=RS ";
    }

    void timeout(int color) {
        calculateScore();
        if (color == BLACK) blackScore = -1;
        else whiteScore = -1;
        log += to_string(color) + "=TO ";
    }

    vector <string> getCaptured() {
        vector <string> captured;
        for (int pos: this->captured[0]) {
            captured.push_back(toCoords(pos));
        }
        return captured;
    }

    void calculateScore() {
        for (int pos = 0; pos < boardRange * boardRange; pos++) {
            if (board[pos] == EMPTY) {
                determineTerritory(pos);
            }
        }

        printBoard();

        blackScore = 0.0;
        whiteScore = komi;
        blackTerritory = "";
        whiteTerritory = "";

        for (int pos = 0; pos < boardRange * boardRange; pos++) {
            if (board[pos] == (MARKER | BLACK)) {
                blackScore += 1;
                blackTerritory += toCoords(pos) + " ";
                board[pos] = EMPTY;
            } else if (board[pos] == (MARKER | WHITE)) {
                whiteScore += 1;
                whiteTerritory += toCoords(pos) + " ";
                board[pos] = EMPTY;
            } else if (board[pos] == MARKER) {
                board[pos] = EMPTY;
            }
        }

        blackScore += captured[WHITE].size();
        whiteScore += captured[BLACK].size();
    }

    string getId() {
        return id;
    }

    void setId(string id) {
        this->id = id;
    }

    int getBlackPlayerId() {
        return blackPlayerId;
    }

    void setBlackPlayerId(int blackPlayerId) {
        this->blackPlayerId = blackPlayerId;
    }

    int getWhitePlayerId() {
        return whitePlayerId;
    }

    void setWhitePlayerId(int whitePlayerId) {
        this->whitePlayerId = whitePlayerId;
    }

    float getBlackScore() {
        return blackScore;
    }

    void setBlackScore(float blackScore) {
        this->blackScore = blackScore;
    }

    float getWhiteScore() {
        return whiteScore;
    }

    void setWhiteScore(float whiteScore) {
        this->whiteScore = whiteScore;
    }

    string getBlackTerritory() {
        return blackTerritory;
    }

    string getWhiteTerritory() {
        return whiteTerritory;
    }

    string getLog() {
        return log;
    }

    int64_t getTimestamp() {
        return timestamp;
    }

    int getBoardSize() {
        return boardSize;
    }

    float getKomi() {
        return komi;
    }

    int getTimeSystem() {
        return timeSystem;
    }

    int getMainTimeSecondsLeft(int color) {
        return mainTimeSeconds[color];
    }

    int getByoyomiPeriodsLeft(int color) {
        return byoyomiPeriods[color];
    }

    int getByoyomiTimeSeconds() {
        return byoyomiTimeSeconds;
    }

    int isRanked() {
        return ranked;
    }

    void setMainTimeSecondsLeft(int color, int seconds) {
        mainTimeSeconds[color] = seconds;
    }

    void setByoyomiPeriodsLeft(int color, int periods) {
        byoyomiPeriods[color] = periods;
    }
};

#endif //GAME_OF_GO_SERVER_GO_ENGINE_HPP
