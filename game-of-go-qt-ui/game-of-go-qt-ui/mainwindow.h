#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "socket.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    Socket *socket;

private slots:
    void onMessageReceived(QString msgtype, QString payload);
};
#endif // MAINWINDOW_H
