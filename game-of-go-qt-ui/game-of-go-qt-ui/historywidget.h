#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include "socket.h"
#include <QWidget>
#include <QVBoxLayout>

namespace Ui {
class HistoryWidget;
}

class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = nullptr);
    ~HistoryWidget();

private slots:
    void onMessageReceived(QString msgtype, QString payload);

    void on_btn_back_clicked();

private:
    Ui::HistoryWidget *ui;
    QVBoxLayout *vBox;
    Socket *socket;
};

#endif // HISTORYWIDGET_H
