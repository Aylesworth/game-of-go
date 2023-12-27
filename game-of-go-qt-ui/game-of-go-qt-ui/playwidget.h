#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include "socket.h"
#include <QWidget>

namespace Ui {
class PlayWidget;
}

class PlayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlayWidget(QWidget *parent = nullptr);
    ~PlayWidget();
    int getSelectedBoardSize();

private slots:
    void on_btn_challenge_clicked();
    void on_btn_cpu_clicked();
    void onBoardSizeChanged(const QString &currentSize);

    void on_pushButton_2_clicked();

private:
    Ui::PlayWidget *ui;
    Socket *socket;
    int selectedBoardSize;
};

#endif // PLAYWIDGET_H
