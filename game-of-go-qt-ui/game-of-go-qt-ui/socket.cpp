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
    delete this;
}

void Socket::sendMessage(QString msgtype, QString payload) {
    char buff[BUFF_SIZE];
    sprintf(buff, "%s L %d\n%s\n", msgtype.toUtf8().constData(), payload.size(), payload.toUtf8().constData());
    int n_sent = send(sockfd, buff, strlen(buff), 0);
    if (n_sent < 0) {
        perror("Error");
        exit(EXIT_FAILURE);
    }
}

void Socket::runReceiveThread() {
    char buff[BUFF_SIZE];
    int n_received;
    while ((n_received = recv(sockfd, buff, BUFF_SIZE - 1, 0)) > 0) {
        buff[n_received] = '\0';
        int split_idx = strcspn(buff, "\n");
        buff[split_idx] = '\0';

        QString header(buff);
        QString payload(buff + split_idx + 1);

        QStringList headerFields = header.split(' ');
        QString msgtype = headerFields[0];

        emit messageReceived(msgtype, payload);

        printf("Received:\n%s\n%s\n", msgtype.toUtf8().constData(), payload.toUtf8().constData());
    }
}

void *Socket::runReceiveThreadWrapper(void *context) {
    pthread_detach(pthread_self());
    reinterpret_cast<Socket *>(context)->runReceiveThread();
    return NULL;
}
