#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include "gameboardwidget.h"
#include "scoreboardwidget.h"
#include "logtablewidget.h"
#include "socket.h"

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
    bool handleCloseRequest();

private slots:
    void onMessageReceived(QString msgtype, QString payload);
    void onGameBoardClicked(QString coords);
    void on_btn_pass_clicked();
    void on_btn_resign_clicked();
    void on_btn_leave_clicked();

private:
    Ui::GameWidget *ui;
    Socket *socket;
    GameBoardWidget *gameBoard;
    ScoreboardWidget *scoreboard;
    LogTableWidget *logTable;
    int myColor;
    bool myTurn;
    bool gameFinished;
    QString lastCoords;
    int lastColor;
};

#endif // GAMEWIDGET_H
