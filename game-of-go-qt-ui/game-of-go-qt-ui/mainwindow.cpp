#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewidget.h"
#include "menuwidget.h"
#include "playwidget.h"
#include "socket.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(new PlayWidget(this));
    socket = Socket::getInstance();
    connect(socket, &Socket::messageReceived, this, &MainWindow::onMessageReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "INVITE") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QString username = params[0];
        int boardSize = params[1].toInt();
        if (QMessageBox::information(
            this,
            "Message",
            "You've got a challenge from " + username + ". Do you want to accept?",
            {QMessageBox::Yes | QMessageBox::No}
        ) == QMessageBox::Yes) {
            socket->sendMessage("INVRES", payload + "ACCEPT\n");
        } else {
            socket->sendMessage("INVRES", payload + "DECLINE\n");
        }
    }

    if (msgtype == "INVRES") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QString username = params[0];
        int boardSize = params[1].toInt();
        QString reply = params[2] == "ACCEPT" ? "accepted" : "declined";
        QMessageBox::information(this, "Message", "Player " + username + " " + reply + " your challenge");
    }
}
