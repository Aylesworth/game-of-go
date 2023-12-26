#include "historywidget.h"
#include "ui_historywidget.h"
#include "historyitemwidget.h"
#include "socket.h"

HistoryWidget::HistoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryWidget)
    , socket(Socket::getInstance())
{
    ui->setupUi(this);

    QWidget* w = new QWidget(this);
    w->setMaximumHeight(999999);
    vBox = new QVBoxLayout(w);
    vBox->setSpacing(20);
    ui->scrollArea->setWidget(w);

    connect(socket, &Socket::messageReceived, this, &HistoryWidget::onMessageReceived);
    socket->sendMessage("HISTRY");
}

HistoryWidget::~HistoryWidget()
{
    delete ui;
}

void HistoryWidget::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "HISTRY") {
        QStringList lines = payload.split("\n", Qt::SkipEmptyParts);
        for (QString line: lines) {
            QStringList params = line.split(" ", Qt::SkipEmptyParts);
            vBox->addWidget(new HistoryItemWidget(
                params[0],
                params[1].toInt(),
                params[2].toInt(),
                params[3],
                params[4].toFloat(),
                params[5].toFloat(),
                params[6].toLongLong(),
                this
            ));
        }
    }
}
