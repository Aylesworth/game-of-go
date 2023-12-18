#include "mainwindow.h"
#include "loginwindow.h"

#include <QApplication>
#include "socket.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWindow w;
    w.show();

    return a.exec();
}
