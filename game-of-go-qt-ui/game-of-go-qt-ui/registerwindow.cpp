#include "registerwindow.h"
#include "ui_registerwindow.h"
#include "loginwindow.h"

#include <QRegularExpressionValidator>
#include <QMessageBox>

RegisterWindow::RegisterWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RegisterWindow)
{
    ui->setupUi(this);
    socket = Socket::getInstance();
    connect(socket, &Socket::messageReceived, this, &RegisterWindow::onMessageReceived);
    connect(ui->btn_register, &QAbstractButton::clicked, this, &RegisterWindow::submit);
    connect(ui->txt_username, &QLineEdit::returnPressed, this, &RegisterWindow::submit);
    connect(ui->txt_password, &QLineEdit::returnPressed, this, &RegisterWindow::submit);
    connect(ui->txt_confirm_password, &QLineEdit::returnPressed, this, &RegisterWindow::submit);
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

void RegisterWindow::submit()
{
    QString username = ui->txt_username->text();
    QString password = ui->txt_password->text();
    QString confirmPassword = ui->txt_confirm_password->text();

    ui->lbl_error->setText("");
    ui->txt_username->setStyleSheet("background-color: white;");
    ui->txt_password->setStyleSheet("background-color: white;");

    QString errorStyle = "background-color: pink;";

    if (username.isEmpty()) {
        ui->txt_username->setStyleSheet(errorStyle);
        ui->lbl_error->setText("Username should not be empty");
        return;
    }

    QRegularExpression regex("^[A-Za-z0-9_]+$");
    QRegularExpressionValidator validator(regex);
    int pos = 0;
    if (validator.validate(username, pos) != QValidator::Acceptable) {
        ui->txt_username->setStyleSheet(errorStyle);
        ui->lbl_error->setText("Username should just contain letters, numbers and underscores");
        return;
    }

    if (password.isEmpty()) {
        ui->txt_password->setStyleSheet(errorStyle);
        ui->lbl_error->setText("Password should not be empty");
        return;
    }

    if (password.size() < 8) {
        ui->txt_password->setStyleSheet(errorStyle);
        ui->lbl_error->setText("Password should contain at least 8 characters");
        return;
    }

    if (confirmPassword != password) {
        ui->txt_confirm_password->setStyleSheet(errorStyle);
        ui->lbl_error->setText("Confirm password does not match");
        return;
    }

    socket->sendMessage("REGIST", username + "\n" + password + "\n");
}


void RegisterWindow::onMessageReceived(QString msgtype, QString payload) {
    if (msgtype == "OK") {
        QMessageBox::information(this, "Information", payload);
        this->hide();
        LoginWindow *w = new LoginWindow();
        w->move(1000, 500);
        w->show();
    } else if (msgtype == "ERROR") {
        ui->lbl_error->setText(payload);
    }
}

void RegisterWindow::on_lbl_login_linkActivated(const QString &link)
{
    this->hide();
    LoginWindow *w = new LoginWindow();
    w->move(1000, 500);
    w->show();
}

