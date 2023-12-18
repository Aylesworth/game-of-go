#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QMainWindow>
#include "socket.h"

namespace Ui {
class RegisterWindow;
}

class RegisterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();
private slots:
    void submit();
    void onMessageReceived(QString msgtype, QString payload);
    void on_lbl_login_linkActivated(const QString &link);

private:
    Ui::RegisterWindow *ui;
    Socket *socket;
};

#endif // REGISTERWINDOW_H
