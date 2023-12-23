#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "gameboardwidget.h"
#include "scoreboardwidget.h"
#include "logtablewidget.h"

#include <QWidget>

namespace Ui {
class GameWidget;
}

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(int boardSize, int myColor, QWidget *parent = nullptr);
    ~GameWidget();

private:
    Ui::GameWidget *ui;
    GameBoardWidget *gameBoard;
    ScoreboardWidget *scoreboard;
    LogTableWidget *logTable;
    int myColor;
    bool myTurn;
};

#endif // GAMEWIDGET_H
