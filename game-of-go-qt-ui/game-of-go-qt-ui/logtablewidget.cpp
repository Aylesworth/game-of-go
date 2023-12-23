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

void LogTableWidget::addRow(QString player, QString move) {
    model->appendRow({new QStandardItem(player), new QStandardItem(move)});
}

void LogTableWidget::removeLastRow() {
    if (model->rowCount() == 0) return;
    model->removeRow(model->rowCount() - 1);
}
