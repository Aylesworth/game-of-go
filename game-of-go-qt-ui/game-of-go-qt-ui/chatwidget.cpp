#include "chatwidget.h"
#include "ui_chatwidget.h"
#include "socket.h"

#include <QVBoxLayout>
#include <QDateTime>
#include <QLabel>
#include <QScrollBar>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWidget)
    , socket(Socket::getInstance())
{
    ui->setupUi(this);

    QWidget* w = new QWidget(this);
    w->setMaximumHeight(999999);
    vBox = new QVBoxLayout(w);
    vBox->setSpacing(10);
    vBox->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->scrollArea->setWidget(w);
    ui->scrollArea->setStyleSheet("background-color: white;");

    connect(socket, &Socket::messageReceived, this, &ChatWidget::onMessageReceived);
    connect(ui->btn_send, &QAbstractButton::clicked, this, &ChatWidget::sendChatMessage);
    connect(ui->txt_chat, &QLineEdit::returnPressed, this, &ChatWidget::sendChatMessage);
}

ChatWidget::~ChatWidget()
{
    delete ui;
}

void ChatWidget::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "CHAT") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QString sender = params[0];
        QString time = QDateTime::fromSecsSinceEpoch(params[1].toLongLong()).toString("hh:mm");
        QString chatMessage = params[2];

        QLabel *label = new QLabel(QString("%1 [%2]: %3").arg(sender).arg(time).arg(chatMessage));
        label->setFont(QFont("Segoe UI", 10));
        label->setWordWrap(true);
        label->setFixedWidth(285);

        vBox->addWidget(label);
        QScrollBar *scrollBar = ui->scrollArea->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
        return;
    }
}

void ChatWidget::sendChatMessage() {
    QString chatMessage = ui->txt_chat->text().trimmed();
    if (chatMessage.isEmpty()) return;

    ui->txt_chat->setText("");
    socket->sendMessage("CHAT", chatMessage + "\n");
}
