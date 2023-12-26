#include "replaywidget.h"
#include "ui_replaywidget.h"

#include "gameboardwidget.h"
#include "scoreboardwidget.h"
#include "logtablewidget.h"

#include <QDebug>

ReplayWidget::ReplayWidget(
        int boardSize,
        int playerColor,
        float blackScore,
        float whiteScore,
        QStringList log,
        QStringList blackTerritory,
        QStringList whiteTerritory,
        QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReplayWidget)
    , gameBoard(new GameBoardWidget(boardSize, 0, this))
    , scoreboard(new ScoreboardWidget(this))
    , logTable(new LogTableWidget(this))
    , playerColor(playerColor)
    , blackScore(blackScore)
    , whiteScore(whiteScore)
    , log(log)
    , blackTerritory(blackTerritory)
    , whiteTerritory(whiteTerritory)
    , prompts(new QString[log.size()])
    , trackMove(new QString[log.size()])
    , index(-1)
    , speedMillis(1000)
{
    ui->setupUi(this);
    ui->leftVBox_2->addWidget(gameBoard);
    ui->rightVBox_2->setSpacing(20);
    ui->rightVBox_2->addWidget(scoreboard);
    ui->rightVBox_2->addWidget(logTable);
    ui->lbl_prompt->setText(QString("You are %1. Black's turn").arg(playerColor == 1 ? "black" : "white"));
    ui->cmb_speed->addItems({"0.25x", "0.5x", "1x", "2x", "3x", "5x", "10x"});
    ui->cmb_speed->setCurrentText("1x");

    timer = new QTimer(this);
    timer->setInterval(speedMillis);

    connect(ui->cmb_speed, &QComboBox::currentTextChanged, this, &ReplayWidget::onCurrentTextChanged);
    connect(ui->btn_play, &QPushButton::clicked, this, &ReplayWidget::play);
    connect(ui->btn_prev, &QPushButton::clicked, this, &ReplayWidget::prev);
    connect(ui->btn_next, &QPushButton::clicked, this, &ReplayWidget::next);
    connect(ui->btn_first, &QPushButton::clicked, this, &ReplayWidget::first);
    connect(ui->btn_last, &QPushButton::clicked, this, &ReplayWidget::last);
    connect(timer, &QTimer::timeout, this, &ReplayWidget::onTimeout);
}

ReplayWidget::~ReplayWidget()
{
    delete ui;
}

void ReplayWidget::onCurrentTextChanged(const QString &currentText) {
    speedMillis = 1000 / currentText.left(currentText.indexOf("x")).toFloat();
    timer->setInterval(speedMillis);
}

void ReplayWidget::updateScore(int color, int difference) {
    if (color == 1) {
        scoreboard->setBlackScore(scoreboard->getBlackScore() + difference);
    } else {
        scoreboard->setWhiteScore(scoreboard->getWhiteScore() + difference);
    }
}

void ReplayWidget::onTimeout() {
    qDebug() << QString("Move %1").arg(index + 1);
    if (!next()) pause();
}

void ReplayWidget::play() {
    ui->btn_play->setText("⏸");
    disconnect(ui->btn_play, &QPushButton::clicked, this, &ReplayWidget::play);
    connect(ui->btn_play, &QPushButton::clicked, this, &ReplayWidget::pause);
    timer->start();
    onTimeout();
    qDebug() << "Playing...";
}

void ReplayWidget::pause() {
    ui->btn_play->setText("▶");
    disconnect(ui->btn_play, &QPushButton::clicked, this, &ReplayWidget::pause);
    connect(ui->btn_play, &QPushButton::clicked, this, &ReplayWidget::play);
    timer->stop();
}


bool ReplayWidget::next() {
    if (++index >= log.size()) {
        index = log.size() - 1;
        return false;
    }

    QString move = log[index];
    QStringList actions = move.split("/", Qt::SkipEmptyParts);
    logTable->addRow(actions[0][0].toLatin1() - '0', actions[0].mid(2));
    // qDebug() << "Add row: " << QString::number(actions[0][0].toLatin1() - '0') << ", " << actions[0].mid(2);

    for (QString action: actions) {
        int color = action[0].toLatin1() - '0';
        char type = action[1].toLatin1();
        QString param = action.mid(2);
        switch (type) {
        case '+': {
            prompts[index] = color == 1 ? "White's turn" : "Black's turn";
            trackMove[index] = param;
            QString lastCoords = index - 1 >= 0 ? trackMove[index - 1] : "";
            int lastColor = (index - 1) % 2 + 1;
            if (index >= 1 && lastCoords != "") {
                gameBoard->drawStone(lastColor, lastCoords, false);
            }
            gameBoard->drawStone(color, param, true);
            ui->lbl_prompt->setText(prompts[index]);
            break;
        }

        case '-': {
            QStringList targets = param.split(",", Qt::SkipEmptyParts);
            gameBoard->removeStones(targets);
            updateScore(3 - color, targets.size());
            break;
        }

        case '=': {
            QString lastCoords = index - 1 >= 0 ? trackMove[index - 1] : "";
            int lastColor = (index - 1) % 2 + 1;
            if (index >= 1 && lastCoords != "") {
                gameBoard->drawStone(lastColor, lastCoords, false);
            }
            if (param == "PA") {
                if (index == log.size() - 1) {
                    prompts[index] = QString("Black %1 : %2 White. %3 wins!").arg(blackScore).arg(whiteScore)
                                         .arg(blackScore > whiteScore ? "Black" : "White");
                    ui->lbl_prompt->setText(prompts[index]);
                    gameBoard->drawTerritory(1, blackTerritory);
                    gameBoard->drawTerritory(2, whiteTerritory);
                } else {
                    prompts[index] = color == 1 ? "Black passes. White's turn" : "White passes. Black's turn";
                    ui->lbl_prompt->setText(prompts[index]);
                }
            } else if (param == "RS") {
                prompts[index] = color == 1 ? "Black resigns. White wins!" : "White resigns. Black wins!";
                ui->lbl_prompt->setText(prompts[index]);
                gameBoard->drawTerritory(1, blackTerritory);
                gameBoard->drawTerritory(2, whiteTerritory);
            }
            break;
        }
        }
    }
    return true;
}

bool ReplayWidget::prev() {
    if (index < 0) return false;

    logTable->removeLastRow();
    if (index == log.size() - 1) {
        gameBoard->removeAllTerritory();
    }

    QString move = log[index];
    QStringList actions = move.split("/", Qt::SkipEmptyParts);
    for (QString action: actions) {
        int color = action[0].toLatin1() - '0';
        char type = action[1].toLatin1();
        QString param = action.mid(2);
        switch (type) {
        case '+': {
            gameBoard->removeStones({param});
            break;
        }
        case '-': {
            QStringList targets = param.split(",", Qt::SkipEmptyParts);
            for (QString target: targets) {
                gameBoard->drawStone(color, target, false);
            }
            updateScore(3 - color, -targets.size());
        }
        }
    }

    if (--index < 0) {
        ui->lbl_prompt->setText(QString("You are %1. Black's turn").arg(playerColor == 1 ? "black" : "white"));
    } else {
        ui->lbl_prompt->setText(prompts[index]);
        if (trackMove[index] != "") {
            gameBoard->drawStone(index % 2 + 1, trackMove[index], true);
        }
    }
    return true;
}

void ReplayWidget::first() {
    pause();
    while (prev());
}

void ReplayWidget::last() {
    pause();
    while (next());
}
