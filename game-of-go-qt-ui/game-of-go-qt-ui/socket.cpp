#include "socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <QString>
#include <QStringList>
#include <QDebug>

QString Socket::serverAddress = "127.0.0.1";
int Socket::serverPort = 8080;

Socket *Socket::instance = new Socket(serverAddress, serverPort);

Socket *Socket::getInstance() {
    return instance;
}

void Socket::setServer(QString serverAddress, int serverPort) {
    instance = new Socket(serverAddress, serverPort);
}

Socket::Socket(QString serverAddress, int serverPort) {
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {  /* calls socket() */
        perror("\nError: ");
        exit(0);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverPort);
    server_addr.sin_addr.s_addr = inet_addr(serverAddress.toUtf8().constData());

    if (::connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    pthread_t tid;
    pthread_create(&tid, NULL, runReceiveThreadWrapper, this);
}

Socket::~Socket() {

}

void Socket::sendMessage(QString msgtype, QString payload) {
    char buff[BUFF_SIZE];
    int n_sent;
    int blocktype_size = 1;
    int payloadlen_size = 4;
    int header_size = msgtype.size() + blocktype_size + payloadlen_size;

    while (header_size + payload.size() > BUFF_SIZE - 1) {
        QString payloadseg = payload.mid(0, BUFF_SIZE - 1 - header_size);
        memset(buff, 0, BUFF_SIZE);
        sprintf(buff, "%s %s %d\n%s",
                msgtype.toStdString().c_str(),
                "M", BUFF_SIZE - 1 - header_size,
                payloadseg.toStdString().c_str());
        n_sent = send(sockfd, buff, strlen(buff), 0);
        if (n_sent < 0) {
            perror("Error");
            exit(EXIT_FAILURE);
        }
        payload = payload.mid(0, BUFF_SIZE - 1 - header_size);
    }

    memset(buff, 0, BUFF_SIZE);
    sprintf(buff, "%s %s %d\n%s",
            msgtype.toStdString().c_str(),
            "L", payload.size(),
            payload.toStdString().c_str());
    n_sent = send(sockfd, buff, strlen(buff), 0);
    if (n_sent < 0) {
        perror("Error");
        exit(EXIT_FAILURE);
    }
}

void Socket::runReceiveThread() {
    char buff[BUFF_SIZE];
    int n_received;
    QString header, payload, msgtype, blocktype, fullpayload = "", remaining = "";
    int payloadlen;
    while (1) {
        do {
            while (remaining.size() > 0) {
                int split_idx = remaining.indexOf("\n");

                header = remaining.left(split_idx);
                payload = remaining.mid(split_idx + 1);

                QStringList headerFields = header.split(' ');
                msgtype = headerFields[0];
                blocktype = headerFields[1];
                payloadlen = headerFields[2].toInt();
                qDebug() << QString("msgtype = %1, blocktype = %2, payloadlen = %3, payload.size() = %4").arg(msgtype).arg(blocktype).arg(payloadlen).arg(payload.size());

                if (payloadlen < payload.size()) {
                    remaining = payload.mid(payloadlen);
                    payload = payload.mid(0, payloadlen);
                    qDebug() << QString("payload = %1, remaining = %2").arg(payload).arg(remaining);
                } else {
                    remaining = "";
                    qDebug() << QString("payload = %1").arg(payload);
                }
                fullpayload += payload;

                if (blocktype == "L") {
                    emit messageReceived(msgtype, fullpayload);
                    fullpayload = "";
                }
            }
            n_received = recv(sockfd, buff, BUFF_SIZE - 1, 0);
            if (n_received <= 0) {
                printf("Server closed\n");
                exit(EXIT_FAILURE);
            }
            buff[n_received] = '\0';
            printf("Received:\n%s\n", buff);

            int split_idx = strcspn(buff, "\n");
            buff[split_idx] = '\0';

            header = QString(buff);
            payload = QString(buff + split_idx + 1);

            QStringList headerFields = header.split(' ');
            msgtype = headerFields[0];
            blocktype = headerFields[1];
            payloadlen = headerFields[2].toInt();
            qDebug() << QString("msgtype = %1, blocktype = %2, payloadlen = %3").arg(msgtype).arg(blocktype).arg(payloadlen);

            if (payloadlen < payload.size()) {
                remaining = payload.mid(payloadlen);
                payload = payload.mid(0, payloadlen);
                qDebug() << QString("payload = %1, remaining = %2").arg(payload).arg(remaining);
            } else {
                remaining = "";
                qDebug() << QString("payload = %1").arg(payload);
            }

            fullpayload += payload;
        } while (blocktype != "L");

        emit messageReceived(msgtype, fullpayload);
        fullpayload = "";
    }
}

void *Socket::runReceiveThreadWrapper(void *context) {
    pthread_detach(pthread_self());
    reinterpret_cast<Socket *>(context)->runReceiveThread();
    return NULL;
}
