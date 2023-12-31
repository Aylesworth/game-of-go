#include "challengewindow.h"
#include "ui_challengewindow.h"
#include "socket.h"
#include "playwidget.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QMainWindow>

ChallengeWindow::ChallengeWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ChallengeWindow)
{
    ui->setupUi(this);
    invitedUsername = "";
    socket = Socket::getInstance();
    connect(socket, &Socket::messageReceived, this, &ChallengeWindow::onMessageReceived);
    connect(ui->list_online, &QListWidget::currentTextChanged, this, &ChallengeWindow::onPlayerChanged);
    connect(MainWindow::getInstance(), &MainWindow::closeChildWindows, this, &ChallengeWindow::hide);
    ui->btn_challenge->setDisabled(true);
    socket->sendMessage("LSTONL");
}

ChallengeWindow::~ChallengeWindow()
{
    delete ui;
}


void ChallengeWindow::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "LSTONL") {
        ui->list_online->clear();
        QStringList entries = payload.split("\n", Qt::SkipEmptyParts);
        for (QString entry: entries) {
            QString username = entry.section(" ", 0, 0);
            QString status = entry.section(" ", 1);
            ui->list_online->addItem(username + " (" + status + ")");
        }
        return;
    }

    if (msgtype == "CHGONL") {
        socket->sendMessage("LSTONL");
        return;
    }

    if (msgtype == "INVRES") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        QString username = params[0];
        QString reply = params[1];
        if (reply == "DECLINE") {
            QMessageBox::information(this, "Message", "Player " + username + " declined your challenge");
            ui->btn_challenge->setText("Challenge");
            ui->btn_challenge->setEnabled(true);
        }
        return;
    }
}

void ChallengeWindow::onPlayerChanged(const QString &currentPlayer) {
    if (currentPlayer.contains("In game")) {
        ui->btn_challenge->setDisabled(true);
    } else {
        ui->btn_challenge->setEnabled(true);
    }
}

void ChallengeWindow::on_btn_challenge_clicked()
{
    QString username = ui->list_online->currentItem()->text();
    username = username.mid(0, username.indexOf('(')).trimmed();
    QString boardSize = QString::number(static_cast<PlayWidget *>(parentWidget())->getSelectedBoardSize());
    socket->sendMessage("INVITE", username + "\n" + boardSize + "\n");
    ui->btn_challenge->setText("Waiting for reply...");
    ui->btn_challenge->setEnabled(false);
    invitedUsername = username;
}

void ChallengeWindow::closeEvent(QCloseEvent *event) {
    if (ui->btn_challenge->text().startsWith("Waiting")) {
        if (QMessageBox::question(
            this,
            "Confirmation",
            "Do you want to cancel your challenge?"
        ) == QMessageBox::Yes) {
            socket->sendMessage("INVCCL", invitedUsername + "\n");
            event->accept();
            this->deleteLater();
        } else {
            event->ignore();
        }
    }
}
