#ifndef HISTORYITEMWIDGET_H
#define HISTORYITEMWIDGET_H

#include "socket.h"
#include <QWidget>

namespace Ui {
class HistoryItemWidget;
}

class HistoryItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryItemWidget(
        QString id,
        int boardSize,
        int color,
        QString opponent,
        float blackScore,
        float whiteScore,
        int64_t time,
        QWidget *parent = nullptr);
    ~HistoryItemWidget();

private slots:
    void onMessageReceived(QString msgtype, QString payload);
    void on_btn_record_clicked();

private:
    Ui::HistoryItemWidget *ui;
    Socket *socket;
    QString id;
    int boardSize;
    int color;
    float blackScore;
    float whiteScore;
};

#endif // HISTORYITEMWIDGET_H
