#include "scoreboardwidget.h"
#include "ui_scoreboardwidget.h"

#include <QStandardItemModel>

ScoreboardWidget::ScoreboardWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScoreboardWidget)
{
    ui->setupUi(this);

    model = new QStandardItemModel;
    model->setColumnCount(2);
    model->setRowCount(1);
    model->setHorizontalHeaderLabels({"BLACK", "WHITE (+6.5)"});

    int rowHeight = 40;
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setFixedHeight(rowHeight);
    ui->tableView->verticalHeader()->setDefaultSectionSize(rowHeight);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    setBlackScore(0);
    setWhiteScore(0);
}

ScoreboardWidget::~ScoreboardWidget()
{
    delete ui;
}

int ScoreboardWidget::getBlackScore() {
    return blackScore;
}

int ScoreboardWidget::getWhiteScore() {
    return whiteScore;
}

void ScoreboardWidget::setBlackScore(int score) {
    blackScore = score;
    QStandardItem *item = new QStandardItem(QString::number(score));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(0, 0, item);
}

void ScoreboardWidget::setWhiteScore(int score) {
    whiteScore = score;
    QStandardItem *item = new QStandardItem(QString::number(score));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(0, 1, item);
}
