#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewidget.h"
#include "menuwidget.h"
#include "socket.h"

#include <QMessageBox>
#include <QDebug>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = Socket::getInstance();
    connect(socket, &Socket::messageReceived, this, &MainWindow::onMessageReceived);
    setCentralWidget(new MenuWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "INVITE") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QString username = params[0];
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
        return;
    }

    if (msgtype == "INVRES") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QString username = params[0];
        // int boardSize = params[1].toInt();
        QString reply = params[2];
        if (reply == "DECLINE")
            QMessageBox::information(this, "Message", "Player " + username + " declined your challenge");
        return;
    }

    if (msgtype == "SETUP") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        int boardSize = params[0].toInt();
        int color = params[1].toInt();
        setCentralWidget(new GameWidget(boardSize, color, this));
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    GameWidget *gameWidget;
    if ((gameWidget = qobject_cast<GameWidget *>(centralWidget())) && !gameWidget->handleCloseRequest()) {
        event->ignore();
        return;
    }
    event->accept();
}
