#include "mainwindow.h"
#include "loginwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWindow w;
    w.move(1000, 500);
    w.show();

    return a.exec();
}
