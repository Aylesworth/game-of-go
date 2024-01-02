#include "logtablewidget.h"
#include "ui_logtablewidget.h"

#include <QStandardItemModel>

LogTableWidget::LogTableWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogTableWidget)
{
    ui->setupUi(this);

    model = new QStandardItemModel;
    model->setColumnCount(2);
    model->setHorizontalHeaderLabels({"PLAYER", "MOVE"});

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

LogTableWidget::~LogTableWidget()
{
    delete ui;
}

void LogTableWidget::addRow(int color, QString coords) {
    QString player = color == 1 ? "BLACK" : "WHITE";
    QString move;
    if (coords == "PA") {
        move = "passes";
    } else if (coords == "RS") {
        move = "resigns";
    } else if (coords == "TO") {
        move = "runs out of time";
    } else {
        move = "plays " + coords;
    }

    model->appendRow({new QStandardItem(player), new QStandardItem(move)});
    ui->tableView->scrollToBottom();
}

void LogTableWidget::removeLastRow() {
    if (model->rowCount() == 0) return;
    model->removeRow(model->rowCount() - 1);
}
