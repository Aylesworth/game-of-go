#ifndef SOCKET_H
#define SOCKET_H

#include <QObject>

#define BUFF_SIZE 2048

class Socket : public QObject
{
    Q_OBJECT
private:
    static Socket *instance;
    static QString serverAddress;
    static int serverPort;
public:
    static Socket *getInstance();
    static void setServer(QString serverAddress, int serverPort);
private:
    int sockfd;
public:
    Socket(QString serverAddress, int serverPort);
    ~Socket();
    void sendMessage(QString msgtype, QString payload = "");
private:
    static void *runReceiveThreadWrapper(void *context);
    void runReceiveThread();
signals:
    void messageReceived(QString msgtype, QString payload);
};

#endif // SOCKET_H
