#include "mainwindow.h"
#include "loginwindow.h"
#include "gameconfigwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWindow w;
    // MainWindow w;
    // GameConfigWindow w;
    w.move(1000, 500);
    w.show();

    return a.exec();
}
