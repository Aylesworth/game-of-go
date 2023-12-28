#ifndef RANKINGSWIDGET_H
#define RANKINGSWIDGET_H

#include "socket.h"
#include <QWidget>
#include <QStandardItemModel>

namespace Ui {
class RankingsWidget;
}

class RankingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RankingsWidget(QWidget *parent = nullptr);
    ~RankingsWidget();

private slots:
    void onMessageReceived(QString msgtype, QString payload);

    void on_btn_back_clicked();

    void on_btn_refresh_clicked();

private:
    Ui::RankingsWidget *ui;
    Socket *socket;
    QStandardItemModel *model;
};

#endif // RANKINGSWIDGET_H
