#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewidget.h"
#include "menuwidget.h"
#include "replaywidget.h"
#include "socket.h"

#include <QMessageBox>
#include <QDebug>
#include <QCloseEvent>
#include <QStackedWidget>

MainWindow *MainWindow::instance = nullptr;

MainWindow *MainWindow::getInstance() {
    if (instance == nullptr) instance = new MainWindow;
    return instance;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , stackedWidget(new QStackedWidget(this))
{
    ui->setupUi(this);
    socket = Socket::getInstance();
    connect(socket, &Socket::messageReceived, this, &MainWindow::onMessageReceived);

    stackedWidget->addWidget(new MenuWidget);
    setCentralWidget(stackedWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "INVITE") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QString username = params[0];

        QMessageBox *questionBox = new QMessageBox(this);
        questionBox->setIcon(QMessageBox::Question);
        questionBox->setWindowTitle("Question");
        questionBox->setText("You've got a challenge from " + username + ". Do you want to accept?");
        questionBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        connect(MainWindow::getInstance(), &MainWindow::closeChildWindows, questionBox, &QMessageBox::hide);
        if (questionBox->exec() == QMessageBox::Yes) {
            socket->sendMessage("INVRES", payload + "ACCEPT\n");
        } else {
            socket->sendMessage("INVRES", payload + "DECLINE\n");
        }
        return;
    }

    if (msgtype == "INVCCL") {
        QMessageBox::information(this, "Message", "This challenge is no longer valid.");
        return;
    }

    if (msgtype == "SETUP") {
        emit closeChildWindows();
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        int boardSize = params[0].toInt();
        int color = params[1].toInt();
        next(new GameWidget(boardSize, color));
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

void MainWindow::swap(QWidget *widget) {
    stackedWidget->removeWidget(stackedWidget->currentWidget());
    stackedWidget->setCurrentIndex(stackedWidget->addWidget(widget));
}

void MainWindow::next(QWidget *widget) {
    stackedWidget->setCurrentIndex(stackedWidget->addWidget(widget));
}

void MainWindow::previous() {
    if (stackedWidget->count() == 1) return;

    QWidget *w;
    stackedWidget->removeWidget(w = stackedWidget->currentWidget());
    delete w;
}
