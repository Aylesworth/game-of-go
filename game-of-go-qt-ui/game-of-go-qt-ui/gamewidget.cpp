#include "gamewidget.h"
#include "ui_gamewidget.h"
#include "gameboardwidget.h"
#include "scoreboardwidget.h"
#include "logtablewidget.h"
#include "mainwindow.h"
#include "socket.h"

#include <unistd.h>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

GameWidget::GameWidget(
    int boardSize,
    int myColor,
    double komi,
    int timeSystem,
    int mainTime,
    int byoyomiTime,
    int byoyomiPeriods,
    QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , myColor(myColor)
    , myTurn(myColor == 1)
    , gameFinished(false)
    , lastCoords("")
    , lastColor(0)
    , timeSystem(timeSystem)
    , byoyomiTime(byoyomiTime)
    , timer(new QTimer(this))
    , socket(Socket::getInstance())
{
    ui->setupUi(this);
    gameBoard = new GameBoardWidget(boardSize, myColor, this);
    scoreboard = new ScoreboardWidget;
    logTable = new LogTableWidget;

    scoreboard->setKomi(komi);
    scoreboard->setBlackCaptures(0);
    scoreboard->setWhiteCaptures(0);
    if (timeSystem == 0) {
        scoreboard->disableTimeControl();
    } else {
        scoreboard->setBlackMainTime(mainTime);
        scoreboard->setWhiteMainTime(mainTime);
        scoreboard->setBlackByoyomiTime(byoyomiTime);
        scoreboard->setWhiteByoyomiTime(byoyomiTime);
        scoreboard->setBlackByoyomiPeriods(byoyomiPeriods);
        scoreboard->setWhiteByoyomiPeriods(byoyomiPeriods);
    }

    ui->leftVBox->addWidget(gameBoard);
    ui->rightVBox->setSpacing(20);
    ui->rightVBox->addWidget(scoreboard);
    ui->rightVBox->addWidget(logTable);

    ui->btn_pass->setEnabled(myTurn);
    ui->lbl_prompt->setText(QString("You are %1. Black's turn").arg(myColor == 1 ? "black" : "white"));
    gameBoard->setStoneShadowDisabled(!myTurn);

    timer->setInterval(1000);

    connect(gameBoard, &GameBoardWidget::clicked, this, &GameWidget::onGameBoardClicked);
    connect(timer, &QTimer::timeout, this, &GameWidget::onTimeout);
    connect(socket, &Socket::messageReceived, this, &GameWidget::onMessageReceived);
}

GameWidget::~GameWidget()
{
    delete ui;
}

void GameWidget::onGameBoardClicked(QString coords) {
    socket->sendMessage("MOVE", QString("%1\n%2\n").arg(myColor).arg(coords));
    if (timeSystem == 1) {
        usleep(1000);
        socket->sendMessage("BYOYOM", QString("%1\n%2\n%3\n").arg(myColor).arg(timingType).arg(timeLeft));
    }
}

void GameWidget::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "MOVE") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        int color = params[0].toInt();
        QString coords = params[1];
        logTable->addRow(color, coords);
        timer->stop();

        if (coords != "PA") {
            gameBoard->drawStone(color, coords, true);
            if (lastCoords != "") {
                gameBoard->drawStone(lastColor, lastCoords, false);
            }
            lastCoords = coords;
            lastColor = color;
            if (params.size() > 2) {
                QStringList capturedList = params[2].split(" ", Qt::SkipEmptyParts);
                gameBoard->removeStones(capturedList);

                if (color == 1) {
                    scoreboard->setBlackCaptures(scoreboard->getBlackCaptures() + capturedList.size());
                } else {
                    scoreboard->setWhiteCaptures(scoreboard->getWhiteCaptures() + capturedList.size());
                }
            }

            myTurn = !myTurn;
            gameBoard->setStoneShadowDisabled(!myTurn);
            ui->btn_pass->setEnabled(myTurn);
            ui->lbl_prompt->setText(color == 1 ? "White's turn" : "Black's turn");
        } else {
            if (lastCoords != "") {
                gameBoard->drawStone(lastColor, lastCoords, false);
            }
            lastCoords = "";
            lastColor = 0;
            myTurn = true;
            gameBoard->setStoneShadowDisabled(false);
            ui->btn_pass->setEnabled(true);
            ui->lbl_prompt->setText(color == 1 ? "Black passes. White's turn" : "White passes. Black's turn");
        }
        return;
    }

    if (msgtype == "MOVERR") {
        ui->lbl_prompt->setText("Invalid move!");
        return;
    }

    if (msgtype == "INTRPT") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        int color = params[0].toInt();
        QString interruptType = params[1];

        if (interruptType == "RESIGN") {
            ui->lbl_prompt->setText(color == 1 ? "Black resigns. White wins!" : "White resigns. Black wins!");
            logTable->addRow(color, "RS");
        } else if (interruptType == "DRAW") {
            if (QMessageBox::question(this, "Question", "Your opponent would like to offer a draw. Do you agree?") == QMessageBox::Yes) {
                socket->sendMessage("INTRES", payload + "ACCEPT\n");
                ui->lbl_prompt->setText("Match drawn by agreement");
                logTable->addRow(3 - color, "DR");
            } else {
                socket->sendMessage("INTRES", payload + "DECLINE\n");
            }
        } else if (interruptType == "RESTART") {
            if (QMessageBox::question(this, "Question", "Your opponent would like to start a new game. Do you agree?") == QMessageBox::Yes) {
                socket->sendMessage("INTRES", payload + "ACCEPT\n");
            } else {
                socket->sendMessage("INTRES", payload + "DECLINE\n");
            }
        } else if (interruptType == "TIMEOUT") {
            ui->lbl_prompt->setText(color == 1 ? "Black ran out of time. White wins!" : "White ran out of time. Black wins!");
            logTable->addRow(color, "TO");
        }
        return;
    }

    if (msgtype == "INTRES") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        int color = params[0].toInt();
        QString interruptType = params[1];
        QString reply = params[2];

        if (interruptType == "DRAW") {
            if (reply == "ACCEPT") {
                ui->lbl_prompt->setText("Match drawn by agreement");
                logTable->addRow(3 - color, "DR");
                QMessageBox::information(this, "Information", "Your opponent has accepted your draw request.");
            } else {
                QMessageBox::information(this, "Information", "Your opponent has declined your draw request.");
            }
        } else if (interruptType == "RESTART") {
            if (reply == "ACCEPT") {
                // QMessageBox::information(this, "Information", "Your opponent has agreed to start a new game.");
            } else if (reply == "DECLINE") {
                QMessageBox::information(this, "Information", "Your opponent has rejected to start a new game.");
            } else if (reply == "LEFT") {
                QMessageBox::information(this, "Information", "Your opponent has left the game.");
            }
        }
        return;
    }

    if (msgtype == "RESULT") {
        if (lastCoords != "") {
            gameBoard->drawStone(lastColor, lastCoords, false);
        }
        lastCoords = "";
        lastColor = 0;

        gameFinished = true;
        myTurn = false;
        gameBoard->setStoneShadowDisabled(true);
        ui->btn_pass->setEnabled(false);
        ui->btn_resign->setEnabled(false);
        ui->btn_draw->setEnabled(false);
        timer->stop();

        QStringList params = payload.split("\n");
        QStringList scores = params[0].split(" ", Qt::SkipEmptyParts);
        float blackScore = scores[0].toFloat();
        float whiteScore = scores[1].toFloat();

        if (params[1].size() > 1) {
            QStringList blackTerritory = params[1].split(" ", Qt::SkipEmptyParts);
            gameBoard->drawTerritory(1, blackTerritory);
        }

        if (params[2].size() > 1) {
            QStringList whiteTerritory = params[2].split(" ", Qt::SkipEmptyParts);
            gameBoard->drawTerritory(2, whiteTerritory);
        }

        if (blackScore >= 0 && whiteScore >= 0) {
            if (blackScore != whiteScore) {
                QString winner = blackScore > whiteScore ? "Black" : "White";
                ui->lbl_prompt->setText(QString("Black %1 : %2 White. %3 wins!").arg(blackScore).arg(whiteScore).arg(winner));
            } else {
                ui->lbl_prompt->setText(QString("Black %1 : %2 White. Tie game!").arg(blackScore).arg(whiteScore));
            }
        }
        return;
    }

    if (msgtype == "BYOYOM") {
        scoreboard->setBlackByoyomiTime(byoyomiTime);
        scoreboard->setWhiteByoyomiTime(byoyomiTime);
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        timingColor = params[0].toInt();
        timingType = params[1];
        timeLeft = params[2].toInt();
        timer->start();
        return;
    }
}

void GameWidget::onTimeout() {
    if (--timeLeft >= 0) {
        if (timingColor == 1) {
            if (timingType == "M") {
                scoreboard->setBlackMainTime(timeLeft);
            } else {
                scoreboard->setBlackByoyomiTime(timeLeft);
            }
        } else {
            if (timingType == "M") {
                scoreboard->setWhiteMainTime(timeLeft);
            } else {
                scoreboard->setWhiteByoyomiTime(timeLeft);
            }
        }
    } else {
        timer->stop();
        if (timingColor == myColor) {
            socket->sendMessage("BYOYOM", QString("%1\n%2\n%3\n").arg(myColor).arg(timingType).arg(0));
        }
        if (timingType == "B") {
            if (timingColor == 1) {
                scoreboard->setBlackByoyomiPeriods(scoreboard->getBlackByoyomiPeriods() - 1);
            } else {
                scoreboard->setWhiteByoyomiPeriods(scoreboard->getWhiteByoyomiPeriods() - 1);
            }
        }
    }
}

void GameWidget::on_btn_pass_clicked()
{
    if (QMessageBox::question(
            this,
            "Confirmation",
            "Are you sure you want to pass?"
    ) == QMessageBox::Yes) {
        if (lastCoords != "") {
            gameBoard->drawStone(lastColor, lastCoords, false);
        }
        lastCoords = "";
        lastColor = 0;

        socket->sendMessage("MOVE", QString("%1\nPA\n").arg(myColor));
        if (timeSystem == 1) {
            usleep(1000);
            socket->sendMessage("BYOYOM", QString("%1\n%2\n%3\n").arg(myColor).arg(timingType).arg(timeLeft));
        }
        myTurn = false;
        gameBoard->setStoneShadowDisabled(true);
        ui->btn_pass->setEnabled(false);
        ui->lbl_prompt->setText(myColor == 1 ? "Black passes. White's turn" : "White passes. Black's turn");
        logTable->addRow(myColor, "PA");
        timer->stop();
    }
}


void GameWidget::on_btn_resign_clicked()
{
    if (QMessageBox::question(
            this,
            "Confirmation",
            "Are you sure you want to resign?"
    ) == QMessageBox::Yes) {
        // if (lastCoords != "") {
        //     gameBoard->drawStone(lastColor, lastCoords, false);
        // }
        // lastCoords = "";
        // lastColor = 0;

        socket->sendMessage("INTRPT", QString("%1\nRESIGN\n").arg(myColor));
        // myTurn = false;
        // gameBoard->setStoneShadowDisabled(true);
        // ui->btn_pass->setEnabled(false);
        // ui->btn_resign->setEnabled(false);
        ui->lbl_prompt->setText(myColor == 1 ? "Black resigns. White wins!" : "White resigns. Black wins!");
        logTable->addRow(myColor, "RS");
        // timer->stop();
    }
}

void GameWidget::on_btn_draw_clicked()
{
    if (QMessageBox::question(
            this,
            "Confirmation",
            "Are you sure you want to offer a draw?"
            ) == QMessageBox::Yes) {
        socket->sendMessage("INTRPT", QString("%1\nDRAW\n").arg(myColor));
    }
}

void GameWidget::on_btn_new_game_clicked()
{
    if (QMessageBox::question(
            this,
            "Confirmation",
            "Do you want to ask your opponent to start a new game?"
            ) == QMessageBox::Yes) {
        socket->sendMessage("INTRPT", QString("%1\nRESTART\n").arg(myColor));
    }
}

bool GameWidget::handleCloseRequest() {
    if (gameFinished) return true;
    if (QMessageBox::question(
            this,
            "Confirmation",
            "Are you sure you want to leave?\nThis will be considered as your defeat."
    ) == QMessageBox::Yes) {
        socket->sendMessage("INTRPT", QString("%1\nRESIGN\n").arg(myColor));
        return true;
    }
    return false;
}

void GameWidget::on_btn_leave_clicked()
{
    if (handleCloseRequest()) {
        socket->sendMessage("RESACK");
        MainWindow::getInstance()->previous();
    }
}



