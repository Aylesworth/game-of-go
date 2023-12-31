#include "gamewidget.h"
#include "ui_gamewidget.h"
#include "gameboardwidget.h"
#include "scoreboardwidget.h"
#include "logtablewidget.h"
#include "mainwindow.h"
#include "socket.h"
#include "menuwidget.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

GameWidget::GameWidget(int boardSize, int myColor, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , myColor(myColor)
    , myTurn(myColor == 1)
    , gameFinished(false)
    , lastCoords("")
    , lastColor(0)
    , socket(Socket::getInstance())
{
    ui->setupUi(this);
    gameBoard = new GameBoardWidget(boardSize, myColor, this);
    scoreboard = new ScoreboardWidget;
    logTable = new LogTableWidget;

    ui->leftVBox->addWidget(gameBoard);
    ui->rightVBox->setSpacing(20);
    ui->rightVBox->addWidget(scoreboard);
    ui->rightVBox->addWidget(logTable);

    ui->btn_pass->setEnabled(myTurn);
    ui->lbl_prompt->setText(QString("You are %1. Black's turn").arg(myColor == 1 ? "black" : "white"));
    gameBoard->setStoneShadowDisabled(!myTurn);

    connect(gameBoard, &GameBoardWidget::clicked, this, &GameWidget::onGameBoardClicked);
    connect(socket, &Socket::messageReceived, this, &GameWidget::onMessageReceived);
}

GameWidget::~GameWidget()
{
    delete ui;
}

void GameWidget::onGameBoardClicked(QString coords) {
    socket->sendMessage("MOVE", QString("%1\n%2\n").arg(myColor).arg(coords));
}

void GameWidget::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "MOVE") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        int color = params[0].toInt();
        QString coords = params[1];
        logTable->addRow(color, coords);

        if (coords != "PA") {
            gameBoard->drawStone(color, coords, true);
            qDebug() << lastCoords;
            if (lastCoords != "") {
                qDebug() << lastCoords;
                // gameBoard->removeStones({lastCoords});
                gameBoard->drawStone(lastColor, lastCoords, false);
            }
            lastCoords = coords;
            lastColor = color;
            if (params.size() > 2) {
                QStringList capturedList = params[2].split(" ", Qt::SkipEmptyParts);
                gameBoard->removeStones(capturedList);

                if (color == 1) {
                    scoreboard->setBlackScore(scoreboard->getBlackScore() + capturedList.size());
                } else {
                    scoreboard->setWhiteScore(scoreboard->getWhiteScore() + capturedList.size());
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
        // QMessageBox::warning(this, "Warning", "Invalid move!");
        ui->lbl_prompt->setText("Invalid move!");
        return;
    }

    if (msgtype == "INTRPT") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        int color = params[0].toInt();
        if (params[1] == "RESIGN") {
            myTurn = false;
            gameBoard->setStoneShadowDisabled(true);
            ui->btn_pass->setEnabled(false);
            ui->btn_resign->setEnabled(false);
            ui->lbl_prompt->setText(color == 1 ? "Black resigns. White wins!" : "White resigns. Black wins!");
            logTable->addRow(color, "RS");
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

        QString winner = blackScore > whiteScore ? "Black" : "White";
        if (blackScore >= 0 && whiteScore >= 0) {
            ui->lbl_prompt->setText(QString("Black %1 : %2 White. %3 wins!").arg(blackScore).arg(whiteScore).arg(winner));
        }
        return;
    }
}

void GameWidget::on_btn_pass_clicked()
{
    if (QMessageBox::question(
            this,
            "Message",
            "Are you sure you want to pass?"
    ) == QMessageBox::Yes) {
        if (lastCoords != "") {
            gameBoard->drawStone(lastColor, lastCoords, false);
        }
        lastCoords = "";
        lastColor = 0;

        socket->sendMessage("MOVE", QString("%1\nPA\n").arg(myColor));
        myTurn = false;
        gameBoard->setStoneShadowDisabled(true);
        ui->btn_pass->setEnabled(false);
        ui->lbl_prompt->setText(myColor == 1 ? "Black passes. White's turn" : "White passes. Black's turn");
        logTable->addRow(myColor, "PA");
    }
}


void GameWidget::on_btn_resign_clicked()
{
    if (QMessageBox::question(
            this,
            "Message",
            "Are you sure you want to resign?"
    ) == QMessageBox::Yes) {
        if (lastCoords != "") {
            gameBoard->drawStone(lastColor, lastCoords, false);
        }
        lastCoords = "";
        lastColor = 0;

        socket->sendMessage("INTRPT", QString("%1\nRESIGN\n").arg(myColor));
        myTurn = false;
        gameBoard->setStoneShadowDisabled(true);
        ui->btn_pass->setEnabled(false);
        ui->btn_resign->setEnabled(false);
        ui->lbl_prompt->setText(myColor == 1 ? "Black resigns. White wins!" : "White resigns. Black wins!");
        logTable->addRow(myColor, "RS");
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

