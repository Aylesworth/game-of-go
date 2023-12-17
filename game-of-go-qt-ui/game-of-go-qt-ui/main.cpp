#include "mainwindow.h"
#include "loginwindow.h"

#include <QApplication>
#include "socket.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // w.show();

    Socket s("127.0.0.1", 8080);
    s.sendMessage("LOGIN", "nda2105\nducanh2003");
    return a.exec();
}
