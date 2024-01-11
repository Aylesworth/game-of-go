#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "registerwindow.h"
#include "mainwindow.h"
#include "menuwidget.h"
#include "socket.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    socket = Socket::getInstance();
    connect(socket, &Socket::messageReceived, this, &LoginWindow::onMessageReceived);
    connect(ui->btn_login, &QAbstractButton::clicked, this, &LoginWindow::submit);
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

    ui->lbl_error->setText("");
    ui->txt_username->setStyleSheet("background-color: white;");
    ui->txt_password->setStyleSheet("background-color: white;");

    QString errorStyle = "background-color: pink;";

    if (username.isEmpty()) {
        ui->txt_username->setStyleSheet(errorStyle);
        ui->lbl_error->setText("Please enter your username");
        return;
    }

    if (password.isEmpty()) {
        ui->txt_password->setStyleSheet(errorStyle);
        ui->lbl_error->setText("Please enter your password");
        return;
    }

    socket->sendMessage("LOGIN", username + "\n" + password + "\n");
}


void LoginWindow::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "OK") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        if (params[0] != "LOGIN") return;
        this->hide();
        MainWindow *w = MainWindow::getInstance();
        w->next(new MenuWidget());
        w->show();
    } else if (msgtype == "ERROR") {
        QStringList params = payload.split("\n", Qt::SkipEmptyParts);
        if (params[0] != "LOGIN") return;
        ui->lbl_error->setText(params[1]);
    }
}


void LoginWindow::on_lbl_register_linkActivated(const QString &link)
{
    this->close();
    RegisterWindow *w = new RegisterWindow();
    w->move(1000, 500);
    w->show();
}

