#include "rankingswidget.h"
#include "ui_rankingswidget.h"
#include "mainwindow.h"
#include "socket.h"

#include <QStandardItemModel>

RankingsWidget::RankingsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RankingsWidget)
    , socket(Socket::getInstance())
{
    ui->setupUi(this);

    model = new QStandardItemModel;
    model->setColumnCount(4);
    model->setHorizontalHeaderLabels({"Ranking", "Player", "ELO", "Rank type"});

    int rowHeight = 40;
    ui->rankings_table->setModel(model);
    ui->rankings_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->rankings_table->horizontalHeader()->setFixedHeight(rowHeight);
    ui->rankings_table->verticalHeader()->setDefaultSectionSize(rowHeight);
    ui->rankings_table->verticalHeader()->setVisible(false);
    ui->rankings_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(socket, &Socket::messageReceived, this, &RankingsWidget::onMessageReceived);

    socket->sendMessage("RANKIN");
}

RankingsWidget::~RankingsWidget()
{
    delete ui;
}

void RankingsWidget::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "RANKIN") {
        model->setRowCount(0);
        QStringList rows = payload.split("\n", Qt::SkipEmptyParts);
        for (QString row: rows) {
            QStringList cols = row.split(" ", Qt::SkipEmptyParts);
            QList<QStandardItem *> items;
            for (QString col: cols) {
                QStandardItem *item = new QStandardItem(col);
                item->setTextAlignment(Qt::AlignCenter);
                items.push_back(item);
            }
            model->appendRow(items);
        }
    }
}

void RankingsWidget::on_btn_back_clicked()
{
    MainWindow::getInstance()->previous();
}


void RankingsWidget::on_btn_refresh_clicked()
{
    socket->sendMessage("RANKIN");
}

