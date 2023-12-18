#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "socket.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    socket = Socket::getInstance();
    connect(socket, &Socket::messageReceived, this, &LoginWindow::onMessageReceived);
    connect(ui->btn_login, &QAbstractButton::clicked, this, &LoginWindow::submit);
    connect(ui->btn_login, &QAbstractButton::pressed, this, &LoginWindow::submit);
    connect(ui->txt_username, &QLineEdit::returnPressed, this, &LoginWindow::submit);
    connect(ui->txt_password, &QLineEdit::returnPressed, this, &LoginWindow::submit);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::submit()
{
    QString username = ui->txt_username->text();
    QString password = ui->txt_password->text();

    ui->label_3->setText("");
    ui->txt_username->setStyleSheet("background-color: white;");
    ui->txt_password->setStyleSheet("background-color: white;");
    if (username.isEmpty()) {
        ui->txt_username->setStyleSheet("background-color: pink;");
        ui->label_3->setText("Please enter your username");
        return;
    }

    if (password.isEmpty()) {
        ui->txt_password->setStyleSheet("background-color: pink;");
        ui->label_3->setText("Please enter your password");
        return;
    }

    socket->sendMessage("LOGIN", ui->txt_username->text() + "\n" + ui->txt_password->text() + "\n");
}


void LoginWindow::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "OK") {
        ui->label_3->setText(payload);
    } else if (msgtype == "ERROR") {
        ui->label_3->setText(payload);
    }
}

