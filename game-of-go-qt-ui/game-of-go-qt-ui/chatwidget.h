#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include "socket.h"
#include <QWidget>
#include <QVBoxLayout>

namespace Ui {
class ChatWidget;
}

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();

private slots:
    void onMessageReceived(QString msgtype, QString payload);
    void sendChatMessage();

private:
    Ui::ChatWidget *ui;
    QVBoxLayout *vBox;
    Socket *socket;
};

#endif // CHATWIDGET_H
