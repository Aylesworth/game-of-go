#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewidget.h"
#include "menuwidget.h"
#include "scoreboardwidget.h"
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
        QStringList timeParams = params[3].split(" ", Qt::SkipEmptyParts);

        QString username = params[0];
        int boardSize = params[1].toInt();
        double komi = params[2].toDouble();
        int timeSystem = timeParams[0].toInt();
        int mainTimeSeconds = timeParams[1].toInt();
        int byoyomiTimeSeconds = timeParams[2].toInt();
        int byoyomiPeriods = timeParams[3].toInt();
        int ranked = params[4].toInt();

        QString details = QString("Board size: %1x%1\nKomi: %2\nTime control: %3\n%4")
                              .arg(boardSize)
                              .arg(komi)
                              .arg(timeSystem == 0 ? "None" :
                                       QString("Byo-yomi (%1m + %2 x %3s)")
                                                                  .arg(mainTimeSeconds / 60)
                                                                  .arg(byoyomiPeriods)
                                                                  .arg(byoyomiTimeSeconds))
                              .arg(ranked == 1 ? "Ranked" : "Not ranked");


        QMessageBox *questionBox = new QMessageBox(this);
        questionBox->setIcon(QMessageBox::Question);
        questionBox->setWindowTitle("Question");
        questionBox->setText("You've got a challenge from " + username + ". Do you want to accept?\n" + details);
        questionBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        connect(MainWindow::getInstance(), SIGNAL(matchSetUp()), questionBox, SLOT(hide()));
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
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QStringList timeParams = params[4].split(" ", Qt::SkipEmptyParts);

        int boardSize = params[1].toInt();
        int color = params[2].toInt();
        double komi = params[3].toDouble();
        int timeSystem = timeParams[0].toInt();
        int mainTimeSeconds = timeParams[1].toInt();
        int byoyomiTimeSeconds = timeParams[2].toInt();
        int byoyomiPeriods = timeParams[3].toInt();
        if (qobject_cast<GameWidget *>(stackedWidget->currentWidget())) {
            swap(new GameWidget(boardSize, color, komi, timeSystem, mainTimeSeconds, byoyomiTimeSeconds, byoyomiPeriods));
        } else {
            next(new GameWidget(boardSize, color, komi, timeSystem, mainTimeSeconds, byoyomiTimeSeconds, byoyomiPeriods));
        }
        emit matchSetUp();
        emit matchSetUp(params[0]);
        return;
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    GameWidget *gameWidget;
    if ((gameWidget = qobject_cast<GameWidget *>(stackedWidget->currentWidget())) && !gameWidget->handleCloseRequest()) {
        event->ignore();
        return;
    }
    event->accept();
}

void MainWindow::swap(QWidget *widget) {
    QWidget* w;
    stackedWidget->removeWidget(w = stackedWidget->currentWidget());
    stackedWidget->setCurrentIndex(stackedWidget->addWidget(widget));
    delete w;
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
