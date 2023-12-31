#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include "socket.h"
#include <QWidget>
#include <QMessageBox>
#include <QTimer>

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
    void onBoardSizeChanged(const QString &currentSize);
    void onMatchSetUp(QString opponent);
    void onTimeout();
    void on_btn_challenge_clicked();
    void on_btn_cpu_clicked();
    void on_btn_quick_clicked();
    void on_btn_leave_clicked();

private:
    Ui::PlayWidget *ui;
    Socket *socket;
    QTimer *timer;
    QMessageBox *waitingBox;
    int waitingSeconds;
    int selectedBoardSize;
};

#endif // PLAYWIDGET_H
