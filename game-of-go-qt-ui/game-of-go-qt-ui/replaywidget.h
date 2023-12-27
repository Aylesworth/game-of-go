#ifndef REPLAYWIDGET_H
#define REPLAYWIDGET_H

#include "gameboardwidget.h"
#include "scoreboardwidget.h"
#include "logtablewidget.h"

#include <QWidget>
#include <QTimer>

namespace Ui {
class ReplayWidget;
}

class ReplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReplayWidget(
        int boardSize,
        int playerColor,
        float blackScore,
        float whiteScore,
        QStringList log,
        QStringList blackTerritory,
        QStringList whiteTerritory,
        QWidget *parent = nullptr);
    ~ReplayWidget();

private slots:
    void on_btn_leave_clicked();
    void onCurrentTextChanged(const QString &currentText);
    void onTimeout();
    void play();
    void pause();
    bool next();
    bool prev();
    void first();
    void last();

private:
    Ui::ReplayWidget *ui;
    GameBoardWidget *gameBoard;
    ScoreboardWidget *scoreboard;
    LogTableWidget *logTable;
    int playerColor;
    float blackScore;
    float whiteScore;
    QStringList blackTerritory;
    QStringList whiteTerritory;
    QStringList log;
    int index;
    QString *prompts;
    QString *trackMove;
    int speedMillis;
    QTimer *timer;
    void updateScore(int color, int difference);
};

#endif // REPLAYWIDGET_H
