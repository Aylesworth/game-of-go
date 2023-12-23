#include "gamewidget.h"
#include "ui_gamewidget.h"
#include "gameboardwidget.h"
#include "scoreboardwidget.h"
#include "logtablewidget.h"

#include <QVBoxLayout>

GameWidget::GameWidget(int boardSize, int myColor, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
{
    ui->setupUi(this);
    gameBoard = new GameBoardWidget(boardSize, this);
    scoreboard = new ScoreboardWidget;
    logTable = new LogTableWidget;

    ui->leftVBox->addWidget(gameBoard);
    ui->rightVBox->setSpacing(20);
    ui->rightVBox->addWidget(scoreboard);
    ui->rightVBox->addWidget(logTable);
}

GameWidget::~GameWidget()
{
    delete ui;
}
