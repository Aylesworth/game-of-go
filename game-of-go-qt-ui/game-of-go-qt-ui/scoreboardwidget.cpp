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
    model->setRowCount(4);
    model->setHorizontalHeaderLabels({"BLACK", "WHITE (+6.5)"});
    model->setVerticalHeaderLabels({"Captures", "Main time", "Byo-yomi", "Periods"});

    int rowHeight = 40;
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setFixedHeight(rowHeight);
    ui->tableView->verticalHeader()->setDefaultSectionSize(rowHeight);
    // ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    setBlackCaptures(0);
    setWhiteCaptures(0);
}

ScoreboardWidget::~ScoreboardWidget()
{
    delete ui;
}

void ScoreboardWidget::disableTimeControl() {
    model->setRowCount(1);
    setFixedHeight(40 * (model->rowCount() + 1));
}

void ScoreboardWidget::disableByoyomi() {
    model->setRowCount(2);
    setFixedHeight(40 * (model->rowCount() + 1));
}


int ScoreboardWidget::getBlackCaptures() {
    return blackCaptures;
}

int ScoreboardWidget::getWhiteCaptures() {
    return whiteCaptures;
}

int ScoreboardWidget::getBlackMainTime() {
    return blackMainTime;
}

int ScoreboardWidget::getWhiteMainTime() {
    return whiteMainTime;
}

int ScoreboardWidget::getBlackByoyomiTime() {
    return blackByoyomiTime;
}

int ScoreboardWidget::getWhiteByoyomiTIme() {
    return whiteByoyomiTime;
}

int ScoreboardWidget::getBlackByoyomiPeriods() {
    return blackByoyomiPeriods;
}

int ScoreboardWidget::getWhiteByoyomiPeriods() {
    return whiteByoyomiPeriods;
}

void ScoreboardWidget::setKomi(double komi) {
    model->setHorizontalHeaderLabels({"BLACK", QString("WHITE (+%1)").arg(komi)});
}

void ScoreboardWidget::setBlackCaptures(int captures) {
    blackCaptures = captures;
    QStandardItem *item = new QStandardItem(QString::number(captures));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(0, 0, item);
}

void ScoreboardWidget::setWhiteCaptures(int captures) {
    whiteCaptures = captures;
    QStandardItem *item = new QStandardItem(QString::number(captures));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(0, 1, item);
}

void ScoreboardWidget::setBlackMainTime(int mainTime) {
    blackMainTime = mainTime;
    QStandardItem *item = new QStandardItem(QString::asprintf("%02d:%02d", mainTime / 60, mainTime % 60));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(1, 0, item);
}

void ScoreboardWidget::setWhiteMainTime(int mainTime) {
    whiteMainTime = mainTime;
    QStandardItem *item = new QStandardItem(QString::asprintf("%02d:%02d", mainTime / 60, mainTime % 60));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(1, 1, item);
}

void ScoreboardWidget::setBlackByoyomiTime(int byoyomiTime) {
    blackByoyomiTime = byoyomiTime;
    QStandardItem *item = new QStandardItem(QString::asprintf("%02d:%02d", byoyomiTime / 60, byoyomiTime % 60));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(2, 0, item);
}

void ScoreboardWidget::setWhiteByoyomiTime(int byoyomiTime) {
    blackByoyomiTime = byoyomiTime;
    QStandardItem *item = new QStandardItem(QString::asprintf("%02d:%02d", byoyomiTime / 60, byoyomiTime % 60));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(2, 1, item);
}

void ScoreboardWidget::setBlackByoyomiPeriods(int byoyomiPeriods) {
    blackByoyomiPeriods = byoyomiPeriods;
    QStandardItem *item = new QStandardItem(QString::number(byoyomiPeriods));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(3, 0, item);
}

void ScoreboardWidget::setWhiteByoyomiPeriods(int byoyomiPeriods) {
    blackByoyomiPeriods = byoyomiPeriods;
    QStandardItem *item = new QStandardItem(QString::number(byoyomiPeriods));
    item->setTextAlignment(Qt::AlignCenter);
    model->setItem(3, 1, item);
}
