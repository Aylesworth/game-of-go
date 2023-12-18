#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include "socket.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

public slots:
    void submit();
    void onMessageReceived(QString msgtype, QString payload);

private:
    Ui::LoginWindow *ui;
    Socket *socket;
};

#endif // LOGINWINDOW_H
